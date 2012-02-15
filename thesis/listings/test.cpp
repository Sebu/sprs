void TrainerMairal::update(MatrixXf& A, MatrixXf& B, Dictionary& D) {
    for(int i=0; i < 1; i++) {
        for(int j=0; j < D.getElementCount(); j++) { 
            MatrixXf a = A.col(j);
            MatrixXf b = B.col(j);
            MatrixXf d = D.getData().col(j);
            if(A.coeff(j,j)==0.0) continue;
            MatrixXf u = ( (1.0/A.coeff(j,j)) * (b-(D.getData()*a)) ) + d;
            D.getData().col(j) = (1.0/fmax(u.norm(),1.0)) * u;
        }
    }
}