
#include "XM2/QuaternionSOA.h"

namespace XM2
{  
#if !XM2_NO_C_IO
//----------------------------------------------------------------------------------------------------------------------
void XQuaternionSOA::fprint32(FILE* fp) const
{
  // ensure not NULL
  XM2_ASSERT(fp);
  XM2_ALIGN_ASSERT(this);

  for(int i=0;i<4;++i)
    ::fprintf(fp,"%f %f %f %f\n",(float)x[i],(float)y[i],(float)z[i],(float)w[i]);
}

//----------------------------------------------------------------------------------------------------------------------
void XQuaternionSOA::fprint64(FILE* fp) const
{
  // ensure not NULL
  XM2_ASSERT(fp);
  XM2_ALIGN_ASSERT(this);

  for(int i=0;i<4;++i)
    ::fprintf(fp,"%lf %lf %lf %lf\n",(double)x[i],(double)y[i],(double)z[i],(double)w[i]);
}

//----------------------------------------------------------------------------------------------------------------------
void XQuaternionSOA::fscan32(FILE* fp)
{
  // ensure not NULL
  XM2_ASSERT(fp);
  XM2_ALIGN_ASSERT(this);

  for(int i=0;i<4;++i)
  {
    #if XM2_USE_FLOAT
    # if XM2_USE_SECURE_SCL
    ::fscanf_s(fp,"%f%f%f%f",&x[i],&y[i],&z[i],&w[i]);
    # else
    ::fscanf(fp,"%f%f%f%f",&x[i],&y[i],&z[i],&w[i]);
    # endif
    #else
    float x_,y_,z_,w_;
    # if XM2_USE_SECURE_SCL
    ::fscanf_s(fp,"%f%f%f%f",&x_,&y_,&z_,&w_);
    # else
    ::fscanf(fp,"%f%f%f%f",&x_,&y_,&z_,&w_);
    # endif
    x[i] = (XReal)x_;
    y[i] = (XReal)y_;
    z[i] = (XReal)z_;
    w[i] = (XReal)w_;
    #endif
  }
}

//----------------------------------------------------------------------------------------------------------------------
void XQuaternionSOA::fscan64(FILE* fp)
{
  // ensure not NULL
  XM2_ASSERT(fp);
  XM2_ALIGN_ASSERT(this);

  for(int i=0;i<4;++i)
  {
    #if !XM2_USE_FLOAT
    # if XM2_USE_SECURE_SCL
    ::fscanf_s(fp,"%lf%lf%lf%lf",&x[i],&y[i],&z[i],&w[i]);
    # else
    ::fscanf(fp,"%lf%lf%lf%lf",&x[i],&y[i],&z[i],&w[i]);
    # endif
    #else
    double x_,y_,z_,w_;
    # if XM2_USE_SECURE_SCL
    ::fscanf_s(fp,"%lf%lf%lf%lf",&x_,&y_,&z_,&w_);
    # else
    ::fscanf(fp,"%lf%lf%lf%lf",&x_,&y_,&z_,&w_);
    # endif
    x[i] = (XReal)x_;
    y[i] = (XReal)y_;
    z[i] = (XReal)z_;
    w[i] = (XReal)w_;
    #endif
  }
}

//----------------------------------------------------------------------------------------------------------------------
void XQuaternionSOA::print32() const
{
  fprint32(stdout);
}

//----------------------------------------------------------------------------------------------------------------------
void XQuaternionSOA::print64() const
{
  fprint64(stdout);
}

//----------------------------------------------------------------------------------------------------------------------
void XQuaternionSOA::fwrite32(FILE* fp,bool flip_bytes) const
{
  // ensure not NULL
  XM2_ASSERT(fp);
  XM2_ALIGN_ASSERT(this);

  #if XM2_USE_FLOAT
  if(flip_bytes)
  {
    XQuaternionSOA temp(*this);
    XM2swap32(&temp,16);
    ::fwrite(&temp,sizeof(XQuaternionSOA),1,fp);
  }
  else
    ::fwrite(this,sizeof(XQuaternionSOA),1,fp);
  #else
  for(int i=0;i<16;++i)
  {
    float temp = (float)data[i];
    ::fwrite(&temp,sizeof(float),1,fp);
  }
  #endif
}

//----------------------------------------------------------------------------------------------------------------------
void XQuaternionSOA::fwrite64(FILE* fp,bool flip_bytes) const
{
  // ensure not NULL
  XM2_ASSERT(fp);
  XM2_ALIGN_ASSERT(this);

  #if !XM2_USE_FLOAT
  if(flip_bytes)
  {
    XQuaternionSOA temp(*this);
    XM2swap64(&temp,16);
    ::fwrite(&temp,sizeof(XQuaternionSOA),1,fp);
  }
  else
    ::fwrite(this,sizeof(XQuaternionSOA),1,fp);
  #else
  for(int i=0;i<16;++i)
  {
    double temp = (double)data[i];
    if(flip_bytes) XM2swap(temp);
    ::fwrite(&temp,sizeof(double),1,fp);
  }
  #endif
}

//----------------------------------------------------------------------------------------------------------------------
void XQuaternionSOA::fread32(FILE* fp,bool flip_bytes)
{
  // ensure not NULL
  XM2_ASSERT(fp);
  XM2_ALIGN_ASSERT(this);

  #if XM2_USE_FLOAT
  ::fread(this,sizeof(XQuaternionSOA),1,fp);
  if(flip_bytes)
    XM2swap32(this,16);
  #else
  for(int i=0;i<16;++i)
  {
    float temp;
    ::fread(&temp,sizeof(float),1,fp);
    if(flip_bytes)
      XM2swap(temp);
    data[i] = (XReal)temp;
  }
  #endif
}

//----------------------------------------------------------------------------------------------------------------------
void XQuaternionSOA::fread64(FILE* fp,bool flip_bytes)
{
  // ensure not NULL
  XM2_ASSERT(fp);
  XM2_ALIGN_ASSERT(this);

  #if !XM2_USE_FLOAT
  ::fread(this,sizeof(XQuaternionSOA),1,fp);
  if(flip_bytes)
    XM2swap64(this,16);
  #else
  for(int i=0;i<16;++i)
  {
    double temp;
    ::fread(&temp,sizeof(double),1,fp);
    if(flip_bytes)
      XM2swap(temp);
    data[i] = (XReal)temp;
  }
  #endif
}
#endif

#if !XM2_NO_STREAM_IO
//----------------------------------------------------------------------------------------------------------------------
XM2EXPORT std::ostream& operator<<(std::ostream& ofs,const XQuaternionSOA& c)
{
  for(int i=0;i<4;++i)
    ofs << c.x[i] << " " << c.y[i] << " " << c.z[i] << " " << c.w[i] << std::endl;
  return ofs;
}
//----------------------------------------------------------------------------------------------------------------------
XM2EXPORT std::istream& operator>>(std::istream& ifs,XQuaternionSOA& c)
{
  for(int i=0;i<4;++i)
    ifs >> c.x[i] >> c.y[i] >> c.z[i] >> c.w[i];
  return ifs;
}
#endif
}
