#include "Util.h"
#include <algorithm>

HWND g_hWndMain = NULL;

void SetMainHWnd(HWND hWnd)
{
	g_hWndMain = hWnd;
}


HWND GetMainHWnd()
{
	return g_hWndMain;
}

////CString GetExePath()
////{
////	TCHAR exePath[MAX_PATH] = {0};
////	::GetModuleFileNameW(AfxGetInstanceHandle(), exePath, MAX_PATH);
////
////	CString strPath = exePath;
////
////	int nPos = strPath.ReverseFind(_T('\\'));
////	if (nPos > 0)
////	{
////		strPath = strPath.Mid(0, nPos + 1);
////		return strPath;
////	}
////
////	else
////	{
////		return _T("");
////	}
////}
////
////CString GetResPath()
////{
////	CString strExe = GetExePath();
////	if (strExe.GetLength() > 0)
////	{
////		CString strResPath = strExe;
////		strResPath += _T("res\\");
////		return strResPath;
////	}
////
////	return _T("");
////}
////
////// UNIcode默认的UTF16编码转为 ansi 在本机是MBCS（实际上是DBCX）   
////std::string UTF16_2_ANSI( const std::wstring& strUTF16 )  
////{  
////	int nANSILength = ::WideCharToMultiByte(CP_ACP,0,strUTF16.c_str(),-1,NULL,0,0,0);  
////	char *pTemp = new char[nANSILength];
////	//std::string strANSI(nANSILength,' ');  
////	int nRet = ::WideCharToMultiByte(CP_ACP,0,strUTF16.c_str(),-1,pTemp,nANSILength,0,0);  
////	ASSERT(0 != nRet);  
////	return pTemp;  
////}  
////
////std::string StrWToStrA(CStringW w)
////{
////	std::wstring str = w.GetString();
////	return UTF16_2_ANSI(str);
////}
////
////// CStringA StrWToStrA(CStringW w)
////// {
//////  	CStringA strRet(w);
//////  	return strRet;
////// }
////
////CStringW StrAToStrW(CStringA w)
////{
////	CStringW strRet(w);
////	return strRet;
////}
////
////
////
////void LoadBmpData(std::string strFileName, uint8** ppData, uint32 & nWidth, uint32 & nHeight)
////{
////    BITMAPFILEHEADER bmpHeader;
////    BITMAPINFOHEADER bmpInfoHeader;
////    FILE* fpBmp = fopen(strFileName.c_str(), "rb");
////    if (!fpBmp)
////    {
////        return;
////    }
////
////    //read the BITMAPFILEHEADER
////    fread(&bmpHeader.bfType,2,1,fpBmp);//文件类型，一定是BM
////    fread(&bmpHeader.bfSize,4,1,fpBmp);//位图文件的大小，单位是字节
////    fread(&bmpHeader.bfReserved1,2,1,fpBmp);//保留字，一定为0
////    fread(&bmpHeader.bfReserved2,2,1,fpBmp);//保留字，一定为0
////    fread(&bmpHeader.bfOffBits,4,1,fpBmp);//偏移量，从BITMAPFILEHEADER结构到位图位
////
////    //read the BITMAPINFOHEADER
////    fread(&bmpInfoHeader.biSize,4,1,fpBmp);//结构所需的字节数
////    fread(&bmpInfoHeader.biWidth,4,1,fpBmp);//位图的宽度，以像素为单位
////    fread(&bmpInfoHeader.biHeight,4,1,fpBmp);//位图的高度，以像素为单位
////    fread(&bmpInfoHeader.biPlanes,2,1,fpBmp);//目标设备的平面数，必须为1
////    fread(&bmpInfoHeader.biBitCount,2,1,fpBmp);//一个像素的位数
////    fread(&bmpInfoHeader.biCompression,4,1,fpBmp);//自下而上的压缩的位图的压缩类型，可以是BI_RGB，BI_RLE8，BI_RLE4，BI_BITFIELDS，BI_JPEG
////    fread(&bmpInfoHeader.biSizeImage,4,1,fpBmp);//指定图像的大小，以字节为单位。BI_RGB位图设置为0
////    fread(&bmpInfoHeader.biXPelsPerMeter,4,1,fpBmp);//指定目标设备的位图水平分辨率，以每米像素为单位
////    fread(&bmpInfoHeader.biYPelsPerMeter,4,1,fpBmp);//指定目标设备的位图垂直分辨率，以每米像素为单位
////    fread(&bmpInfoHeader.biClrUsed,4,1,fpBmp);//指定实际应用于位图中的颜色表中的颜色索引数
////    fread(&bmpInfoHeader.biClrImportant,4,1,fpBmp);//指定用于显示位图需要的颜色索引数。若为0，则所有颜色都需要。
////
////    // read bmp data
////    uint8 *bmpData = new uint8[bmpHeader.bfSize];
////    fseek(fpBmp,bmpHeader.bfOffBits,0);
////    fread(bmpData,1,bmpHeader.bfSize,fpBmp);
////    uint8 *tempData = new uint8[bmpHeader.bfSize];//bmp图像是倒着的
////    int count=0;
////    for(int i = bmpInfoHeader.biHeight - 1;i>= 0;i--)
////    {
////        for(int j = 0;j<bmpInfoHeader.biWidth*3;j+=3)
////        {
////            tempData[count++] = bmpData[i*bmpInfoHeader.biWidth*3 + j];
////            tempData[count++] = bmpData[i*bmpInfoHeader.biWidth*3 + j+1];
////            tempData[count++] = bmpData[i*bmpInfoHeader.biWidth*3 + j+2];
////        }
////    }
////
////    (*ppData) = new uint8[bmpInfoHeader.biWidth * bmpInfoHeader.biHeight * 3 / 2];
////    ConvertRGB2YUV(bmpInfoHeader.biWidth, bmpInfoHeader.biHeight, tempData, *ppData);
////    nWidth = bmpInfoHeader.biWidth;
////    nHeight = bmpInfoHeader.biHeight;
////    delete bmpData;
////    delete tempData;
////}
////
////void AddImg_I420(uint8* psrc, int32 width, int32 height, uint8* pmark, int32 cx, int32 cy)
////{
////    if (!psrc || !pmark || !width || !height){
////        return;
////    }
////    int32 offset_x = 10;
////    int32 offset_y = 10;
////    int32 orgi_width = (width>>1)<<1;
////    int32 orgi_height = (height>>1)<<1;
////    int32 mark_width = (cx>>1)<<1;
////    int32 mark_height = (cy>>1)<<1;
////
////    uint8* py = NULL, *pu = NULL, *pv = NULL;
////    py = psrc;
////    pu = psrc + orgi_width*orgi_height;
////    pv = psrc + orgi_width*orgi_height*5/4;
////
////    uint8* pmy = NULL, *pmu = NULL, *pmv = NULL;
////    pmy = pmark;
////    pmu = pmark + mark_width*mark_height;
////    pmv = pmark + mark_width*mark_height*5/4;
////
////    int offset = 0;
////    int uvoffset = 0;
////    for (int i = 0; i < mark_height; i ++)
////    {
////        offset = (offset_y + i) * orgi_width + offset_x;
////        if (i%2 == 0){
////            uvoffset = (offset_y + i) * orgi_width / 4 + offset_x / 2;
////        }		
////        int16 p, u, v;
////        for (int j = 0; j < mark_width; j ++)
////        {
////            p = py[offset + j] + pmy[i * mark_width + j];		
////            py[offset+j] = p > 255 ? 255 : p;
////
////            if (i%2 == 1 || j%2 ==1){
////                continue;
////            }
////
////            int s_index = uvoffset + j/2;
////            int a_index = i*mark_width/4 + j/2;
////            if (pmu[a_index] == 128 && pmv[a_index] == 128){
////                //灰度图像的UV分量为128，如果UV为128的时候不叠加UV分量
////                continue;
////            }
////            u = pu[s_index] + pmu[a_index];
////            pu[s_index] = u > 255 ? 255 : u;
////
////            v = pv[s_index] + pmv[a_index];
////            pv[s_index] = v > 255 ? 255 : v;
////        }
////    }
////}
////
////
////int ConvertRGB2YUV(int32 w,int32 h,uint8 *bmp,uint8 *yuv)
////{
////    // Conversion from RGB to YUV420
////    int32 RGB2YUV_YR[256], RGB2YUV_YG[256], RGB2YUV_YB[256];
////    int32 RGB2YUV_UR[256], RGB2YUV_UG[256], RGB2YUV_UBVR[256];
////    int32 RGB2YUV_VG[256], RGB2YUV_VB[256];
////
////    {
////        int32 i;
////
////        for (i = 0; i < 256; i++) RGB2YUV_YR[i] = (float)65.481 * (i<<8);
////        for (i = 0; i < 256; i++) RGB2YUV_YG[i] = (float)128.553 * (i<<8);
////        for (i = 0; i < 256; i++) RGB2YUV_YB[i] = (float)24.966 * (i<<8);
////        for (i = 0; i < 256; i++) RGB2YUV_UR[i] = (float)37.797 * (i<<8);
////        for (i = 0; i < 256; i++) RGB2YUV_UG[i] = (float)74.203 * (i<<8);
////        for (i = 0; i < 256; i++) RGB2YUV_VG[i] = (float)93.786 * (i<<8);
////        for (i = 0; i < 256; i++) RGB2YUV_VB[i] = (float)18.214 * (i<<8);
////        for (i = 0; i < 256; i++) RGB2YUV_UBVR[i] = (float)112 * (i<<8);
////    }
////    
////
////    uint8 *u,*v,*y,*uu,*vv;
////    uint8 *pu1,*pu2,*pu3,*pu4;
////    uint8 *pv1,*pv2,*pv3,*pv4;
////    uint8 *r,*g,*b;
////    int i,j;
////
////    uu=(uint8*)malloc(w*h);
////    vv=(uint8*)malloc(w*h);
////
////    if(uu==NULL || vv==NULL)
////        return 0;
////    y=yuv;
////    u=uu;
////    v=vv;
////    // Get r,g,b pointers from bmp image data....
////    r=bmp;
////    g=bmp+1;
////    b=bmp+2;
////    //Get YUV values for rgb values...
////    for(i=0;i<h;i++)
////    {
////        for(j=0;j<w;j++)
////        {
////            *y++=( RGB2YUV_YR[*r] +RGB2YUV_YG[*g]+RGB2YUV_YB[*b]+1048576)>>16;
////            *u++=(-RGB2YUV_UR[*r] -RGB2YUV_UG[*g]+RGB2YUV_UBVR[*b]+8388608)>>16;
////            *v++=( RGB2YUV_UBVR[*r]-RGB2YUV_VG[*g]-RGB2YUV_VB[*b]+8388608)>>16;
////            r+=3;
////            g+=3;
////            b+=3;
////        }
////    }
////    // Now sample the U & V to obtain YUV 4:2:0 format
////    // Sampling mechanism...
////    /* @ -> Y
////    # -> U or V
////    @ @ @ @
////    # #
////    @ @ @ @
////    @ @ @ @
////    # #
////    @ @ @ @
////    */
////
////    // Get the right pointers...
////    u=yuv+w*h;
////    v=u+(w*h)/4;
////
////    // For U
////    pu1=uu;
////    pu2=pu1+1;
////    pu3=pu1+w;
////    pu4=pu3+1;
////
////    // For V
////    pv1=vv;
////    pv2=pv1+1;
////    pv3=pv1+w;
////    pv4=pv3+1;
////
////    // Do sampling....
////    for(i=0;i<h;i+=2)
////    {
////        for(j=0;j<w;j+=2)
////        {
////            *u++=(*pu1+*pu2+*pu3+*pu4)>>2;
////            *v++=(*pv1+*pv2+*pv3+*pv4)>>2;
////            pu1+=2;
////            pu2+=2;
////            pu3+=2;
////            pu4+=2;
////
////            pv1+=2;
////            pv2+=2;
////            pv3+=2;
////            pv4+=2;
////        }
////
////        pu1+=w;
////        pu2+=w;
////        pu3+=w;
////        pu4+=w;
////
////        pv1+=w;
////        pv2+=w;
////        pv3+=w;
////        pv4+=w;
////
////    }
////
////    free(uu);
////    free(vv);
////
////    return 1;
////}
////
////void _CopyBits2Tex_None_0(uint8* pDst, uint8* pSrc, UINT uLen, const SIZE & size)
////{
////  memcpy(pDst, pSrc,uLen);
////}
////
////void _CopyBits2Tex_None_270(uint8* pDst, uint8* pSrc, UINT uLen, const SIZE & size)
////{
////  int n = 0;
////  int linesize = size.cx * 3;
////
////  for(int x = size.cx - 1; x >= 0; x--) {
////     for(int y = 0; y < size.cy; y ++) {
////      memcpy(pDst + n,  pSrc + linesize * y + 3 * x, 3);
////      n += 3;
////    }
////  }
////}
////
////void _CopyBits2Tex_None_180(uint8* pDst, uint8* pSrc, UINT uLen, const SIZE & size)
////{
////  int n = 0;
////  int linesize = size.cx * 3;
////
////  for(int x = 0; x < size.cx; x ++) {
////    for(int y = 0; y < size.cy; y ++) {
////      memcpy(pDst + linesize * (size.cy - y - 1) +  3 * x,  pSrc + linesize * y +  3 * (size.cx - x - 1), 3);
////    }
////  }
////}
////
////void _CopyBits2Tex_None_90(uint8* pDst, uint8* pSrc, UINT uLen, const SIZE & size)
////{
////  int n = 0;
////  int linesize = size.cx * 3;
////
////  for(int x = 0; x < size.cx; x++) {
////    for(int y = size.cy - 1; y >= 0; y--) {
////      memcpy(pDst + n,  pSrc + linesize * y + 3 * x, 3);
////      n += 3;
////    }
////  }
////}
////
////bool _ResizeWithMendBlack(uint8* pDst, uint8* pSrc, UINT uDstLen, UINT uSrcLen, const SIZE & dstSize, const SIZE & srcSize, UINT bpp)
////{
////  if(!pDst || !pSrc)
////    return false;
////
////  if(uDstLen == 0 || uDstLen != dstSize.cx * dstSize.cy * bpp)
////    return false;
////
////  if(uSrcLen == 0 || uSrcLen != srcSize.cx * srcSize.cy * bpp)
////    return false;
////
////  if(dstSize.cx < srcSize.cx)
////    return false;
////
////  if(dstSize.cy < srcSize.cy)
////    return false;
////
////  UINT dstLineblockSize = dstSize.cx * bpp;
////  UINT srcLineblockSize = srcSize.cx * bpp;
////
////  int mendCxLeftEnd = dstSize.cx > srcSize.cx ? (dstSize.cx - srcSize.cx) / 2 : 0;
////  int mendCyToEnd = dstSize.cy > srcSize.cy ? (dstSize.cy - srcSize.cy) / 2 : 0;
////
////	for(int y = 0; y < dstSize.cy; y++) 
////  {
////    if( y >= mendCyToEnd && y < mendCyToEnd + srcSize.cy)
////		{
////			if(mendCxLeftEnd > 0) 
////        memcpy(pDst + (bpp * mendCxLeftEnd) , pSrc, srcLineblockSize);
////      else
////        memcpy(pDst, pSrc, srcLineblockSize);
////			pSrc += srcLineblockSize;
////		}
////    pDst += dstLineblockSize;
////  }
////}