#include "patch.h"
#include "cv_ext.h"

Patch::Patch(IplImage* image)
    : _histMean(0.0f), _image(image), _orientHist(image, 36)
{
  setHistMean( histogram_mean(_image) );
}
