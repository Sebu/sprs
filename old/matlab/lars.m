function beta = lars(X, y, method, stop, useGram, Gram, trace)
% LARS  The LARS algorithm for performing LAR or LASSO.
%    BETA = LARS(X, Y) performs least angle regression on the variables in
%    X to approximate the response Y. Variables X are assumed to be
%    normalized (zero mean, unit length), the response Y is assumed to be
%    centered.
%    BETA = LARS(X, Y, METHOD), where METHOD is either 'LARS' or 'LARS'  
%    determines whether least angle regression or lasso regression should
%    be performed. 
%    BETA = LARS(X, Y, METHOD, STOP) with nonzero STOP will perform least
%    angle or lasso regression with early stopping. If STOP is negative,
%    STOP is an integer that determines the desired number of variables. If
%    STOP is positive, it corresponds to an upper bound on the L1-norm of
%    the BETA coefficients.
%    BETA = LARS(X, Y, METHOD, STOP, USEGRAM) specifies whether the Gram
%    matrix X'X should be calculated (USEGRAM = 1) or not (USEGRAM = 0).
%    Calculation of the Gram matrix is suitable for low-dimensional
%    problems. By default, the Gram matrix is calculated.
%    BETA = LARS(X, Y, METHOD, STOP, USEGRAM, GRAM) makes it possible to
%    supply a pre-computed Gram matrix. Set USEGRAM to 1 to enable. If no
%    Gram matrix is available, exclude argument or set GRAM = [].
%    BETA = LARS(X, Y, METHOD, STOP, USEGRAM, GRAM, TRACE) with nonzero
%    TRACE will print the adding and subtracting of variables as all
%    LARS/lasso solutions are found.
%    Returns BETA where each row contains the predictor coefficients of
%    one iteration. A suitable row is chosen using e.g. cross-validation,
%    possibly including interpolation to achieve sub-iteration accuracy.
%


%% Input checking
% Set default values.
lasso = 1;


%% LARS variable setup
[n p] = size(X);
nvars = min(n-1,p); % 
maxk = 8*nvars; % Maximum number of iterations

if stop == 0
  beta = zeros(2*nvars, p);
elseif stop < 0
  beta = zeros(2*round(-stop), p);
else
  beta = zeros(100, p);
end
mu = zeros(n, 1); % current "position" as LARS travels towards lsq solution
I = 1:p; % inactive set
A = []; % active set


Gram = X'*X; 


lassocond = 0; % LASSO condition boolean
stopcond = 0; % Early stopping condition boolean
k = 0; % Iteration count
vars = 0; % Current number of variables

%% LARS main loop
while vars < nvars && ~stopcond && k < maxk
  k = k + 1;
  c = X'*(y - mu);
  [C j] = max(abs(c(I)));
  j = I(j);

  if ~lassocond % if a variable has been dropped, do one iteration with this configuration (don't add new one right away)
    A = [A j];
    I(I == j) = [];
    vars = vars + 1;
  end

  s = sign(c(A)); % get the signs of the correlations

  S = s*ones(1,vars);
  GA1 = inv(Gram(A,A).*S'.*S)*ones(vars,1);
  AA = 1/sqrt(sum(GA1));
  w = AA*GA1.*s; % weights applied to each active variable to get equiangular direction

  u = X(:,A)*w; % equiangular direction (unit vector)
  
  if vars == nvars % if all variables active, go all the way to the lsq solution
    gamma = C/AA;
  else
    a = X'*u; % correlation between each variable and eqiangular vector
    temp = [(C - c(I))./(AA - a(I)); (C + c(I))./(AA + a(I))];
    gamma = min([temp(temp > 0); C/AA]);
  end

  % LASSO modification
  if lasso
    lassocond = 0;
    temp = -beta(k,A)./w';
    [gamma_tilde] = min([temp(temp > 0) gamma]);
    j = find(temp == gamma_tilde);
    if gamma_tilde < gamma,
      gamma = gamma_tilde;
      lassocond = 1;
    end
  end

  mu = mu + gamma*u;
  if size(beta,1) < k+1
    beta = [beta; zeros(size(beta,1), p)];
  end
  beta(k+1,A) = beta(k,A) + gamma*w';

  % Early stopping at specified bound on L1 norm of beta
  if stop > 0
    t2 = sum(abs(beta(k+1,:)));
    if t2 >= stop
      t1 = sum(abs(beta(k,:)));
      s = (stop - t1)/(t2 - t1); % interpolation factor 0 < s < 1
      beta(k+1,:) = beta(k,:) + s*(beta(k+1,:) - beta(k,:));
      stopcond = 1;
    end
  end
  
  % If LASSO condition satisfied, drop variable from active set
  if lassocond == 1
    I = [I A(j)];
    A(j) = [];
    vars = vars - 1;
  end
  
  % Early stopping at specified number of variables
  if stop < 0
    stopcond = vars >= -stop;
  end
end

% trim beta
if size(beta,1) > k+1
  beta(k+2:end, :) = [];
end

if k == maxk
  disp('LARS warning: Forced exit. Maximum number of iteration reached.');
end


