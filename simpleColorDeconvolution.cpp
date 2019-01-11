
cv::Mat ColourDeconvolution::process(cv::Mat I, const string stainingType, int vector01, int vector02, int vector03, int vector11, int vector12, int vector13, int vector01_asap, int vector02_asap, int vector03_asap, int vector11_asap, int vector12_asap, int vector13_asap) const
{
    //=======STEP 1=======
    // rgbI <-- RGB channels of (I) with 3 channels & 64-bit precision
	//uint64 a = getTime();
    Mat rgbI;
    if (I.type() == CV_64FC3)
        I.convertTo(I, CV_32FC3);
    // Careful! Image matrix read in OpenCV is in BGR color.
    cvtColor(I, rgbI, CV_BGR2RGB);

	/*cout << "CD t1 = " << getTime() - a << " ms" << endl;
	a = getTime();*/

    switch (rgbI.type())
    {
    case CV_8UC3:
        rgbI.convertTo(rgbI, CV_64FC3, 1.0 / 255, 1.0 / 255.0);
        break;
    case CV_64FC3:
        break;
    case CV_32FC3:
        rgbI.convertTo(rgbI, CV_64FC3, 1, 1.0 / 255.0);
        break;
    default:
        // just for check in test
        cout << "Type of image doesn't belong to {8U,32F,64F}" << endl;
        break;
    }
	/*cout << "CD t2 = " << getTime() - a << " ms" << endl;
	a = getTime();*/

	Mat odRGB = rgbI;

    log(odRGB, odRGB);
	odRGB *= -255.0 / log(255.0);

	/*cout << "CD t3 = " << getTime() - a << " ms" << endl;
	a = getTime();*/
    //normalise vector length

    Mat M = normalizeStaining(stainingType, vector01, vector02, vector03, vector11, vector12, vector13, vector01_asap, vector02_asap, vector03_asap, vector11_asap, vector12_asap, vector13_asap);
    // D <-- M^-1
    Mat D = M.inv();
    //======STEP 4=======
    // odHEB <-- optical density of HEB image
    //		 <-- D^T * odRGB
    Mat odHEB;
    D = D.t();
    transform(odRGB, odHEB, D);

	/*cout << "CD t4 = " << getTime() - a << " ms" << endl;
	a = getTime();*/

	Mat hebI = (255 - odHEB)*(log(255.0) / 255.0);
    exp(hebI, hebI);

	/*cout << "CD t5 = " << getTime() - a << " ms" << endl;
	a = getTime();*/
    //=======STEP 6=======
    // Before thresholding, convert (hebI) to 32-bit matrix
    // because cv::threshold can handle only 8-bit & 32-bit
    //hebI.convertTo(hebI, CV_32FC3);
    //threshold(hebI, hebI, 255, 255, THRESH_TRUNC);
    //hebI += 0.5;
    Mat output;
    // here is no need to multiply 255
    hebI.convertTo(output, CV_8UC3);

	//cout << "CD t6 = " << getTime() - a << " ms" << endl;
    //======RETURN=======
	
	/*Mat writeOutput;
	resize(output, writeOutput, Size(), 1. / 4, 1. / 4, INTER_AREA);
    return output;
}

cv::Mat ColourDeconvolution::normalizeStaining(const std::string stainingType, int vector01, int vector02, int vector03, int vector11, int vector12, int vector13, int vector01_asap, int vector02_asap, int vector03_asap, int vector11_asap, int vector12_asap, int vector13_asap) const
{
    _vd MODx(3,0.0);
    _vd MODy(3,0.0);
    _vd MODz(3,0.0);
    _vd cosx(3,0.0);
    _vd cosy(3,0.0);
    _vd cosz(3,0.0);
    _vd len(3,0.0);

	if (stainingType == "custom")
	{
		if(customMODx.empty() || customMODy.empty() || customMODz.empty()) {
			cerr << "Error in ColourDeconvolution: custom staining selected without setting parameters" << endl;
		}
		MODx = customMODx;
		MODy = customMODy;
		MODz = customMODz;
	}
  
    for (int i=0; i<3; i++)
    {

        cosx[i]=cosy[i]=cosz[i]=0.0;
        len[i]=std::sqrt(MODx[i]*MODx[i] + MODy[i]*MODy[i] + MODz[i]*MODz[i]);
        if (len[i] != 0.0)
        {
            cosx[i]= MODx[i]/len[i];
            cosy[i]= MODy[i]/len[i];
            cosz[i]= MODz[i]/len[i];
        }
    }

    if (cosx[1]==0.0)
    {
        //2nd colour is unspecified
        if (cosy[1]==0.0)
        {
            if (cosz[1]==0.0)
            {
                cosx[1]=cosz[0];
                cosy[1]=cosx[0];
                cosz[1]=cosy[0];
            }
        }
    }

    if (cosx[2]==0.0){ // 3rd colour is unspecified
        if (cosy[2]==0.0)
        {
            if (cosz[2]==0.0)
            {
                if ((cosx[0]*cosx[0] + cosx[1]*cosx[1])> 1){
                    cosx[2]=0.0;
                }
                else {
                    cosx[2]=std::sqrt(1.0-(cosx[0]*cosx[0])-(cosx[1]*cosx[1]));
                }

                if ((cosy[0]*cosy[0] + cosy[1]*cosy[1])> 1){
                    cosy[2]=0.0;
                }
                else {
                    cosy[2]=std::sqrt(1.0-(cosy[0]*cosy[0])-(cosy[1]*cosy[1]));
                }

                if ((cosz[0]*cosz[0] + cosz[1]*cosz[1])> 1){
                    cosz[2]=0.0;
                }
                else {
                    cosz[2]=std::sqrt(1.0-(cosz[0]*cosz[0])-(cosz[1]*cosz[1]));
                }
            }
        }
    }

    double leng = std::sqrt(cosx[2]*cosx[2] + cosy[2]*cosy[2] + cosz[2]*cosz[2]);

    cosx[2]= cosx[2]/leng;
    cosy[2]= cosy[2]/leng;
    cosz[2]= cosz[2]/leng;

    for (int i=0; i<3; i++)
    {
        if (cosx[i] == 0.0)
            cosx[i] = 0.001;
        if (cosy[i] == 0.0)
            cosy[i] = 0.001;
        if (cosz[i] == 0.0)
            cosz[i] = 0.001;
    }

    Mat M = (Mat_<float>(3, 3) <<
        cosx[0], cosy[0], cosz[0],
        cosx[1], cosy[1], cosz[1],
        cosx[2], cosy[2], cosz[2]);
        
return M;