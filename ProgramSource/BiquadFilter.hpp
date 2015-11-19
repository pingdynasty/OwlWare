#ifndef __BiquadFilter_h__
#define __BiquadFilter_h__

#include "FloatArray.h"

class FilterStage {
public:
  FloatArray coefficients;
  FloatArray state;
  static const float BESSEL_Q;
  static const float SALLEN_KEY_Q;
  static const float BUTTERWORTH_Q;

  FilterStage(FloatArray co, FloatArray st) : coefficients(co), state(st){}

  void setLowPass(float fc, float q){
    setLowPass(coefficients, fc, q);
  }

  void setHighPass(float fc, float q){
    setHighPass(coefficients, fc, q);
  }
  
  void setBandPass(float fc, float q){
    setBandPass(coefficients, fc, q);
  }
  
  void setNotch(float fc, float q){
    setNotch(coefficients, fc, q);
  }
  
  void setPeak(float fc, float q, float gain){
    setPeak(coefficients, fc, q, gain);
  }
  void setLowShelf(float fc, float gain){
    setLowShelf(coefficients, fc, gain);
  }
  void setHighShelf(float fc, float gain){
    setHighShelf(coefficients, fc, gain);
  }
  void setCoefficients(FloatArray newCoefficients){
    ASSERT(coefficients.getSize()==newCoefficients.getSize(), "wrong size");
    coefficients.copyFrom(newCoefficients);
  }  

  FloatArray getCoefficients(){
    return coefficients;
  }
  FloatArray getState(){
    return state;
  }

  static void setLowPass(float* coefficients, float fc, float q){
    float omega = M_PI*fc/2;
    float K = tanf(omega);
    float norm = 1 / (1 + K / q + K * K);
    coefficients[0] = K * K * norm;
    coefficients[1] = 2 * coefficients[0];
    coefficients[2] = coefficients[0];
    coefficients[3] = - 2 * (K * K - 1) * norm;
    coefficients[4] = - (1 - K / q + K * K) * norm;
  }

  static void setHighPass(float* coefficients, float fc, float q){
    float omega = M_PI*fc/2;
    float K = tanf(omega);
    float norm = 1 / (1 + K / q + K * K);
    coefficients[0] = 1 * norm;
    coefficients[1] = -2 * coefficients[0];
    coefficients[2] = coefficients[0];
    coefficients[3] = - 2 * (K * K - 1) * norm;
    coefficients[4] = - (1 - K / q + K * K) * norm;
  }

  static void setBandPass(float* coefficients, float fc, float q){
    float omega = M_PI*fc/2;
    float K = tanf(omega);
    float norm = 1 / (1 + K / q + K * K);
    coefficients[0] = K / q * norm;
    coefficients[1] = 0;
    coefficients[2] = -coefficients[0];
    coefficients[3] = - 2 * (K * K - 1) * norm;
    coefficients[4] = - (1 - K / q + K * K) * norm;
  }

  static void setNotch(float* coefficients, float fc, float q){
    float omega = M_PI*fc/2;
    float K = tanf(omega);
    float norm = 1 / (1 + K / q + K * K);
    coefficients[0] = (1 + K * K) * norm;
    coefficients[1] = 2 * (K * K - 1) * norm;
    coefficients[2] = coefficients[0];
    coefficients[3] = - coefficients[1];
    coefficients[4] = - (1 - K / q + K * K) * norm;
  }

  static void setPeak(float* coefficients, float fc, float q, float gain){
    float omega = M_PI*fc/2;
    float K = tanf(omega);
    float V = abs(gain-0.5)*60 + 1; // Gain
    float norm;
    if (gain >= 0.5) {
      norm = 1 / (1 + 1/q * K + K * K);
      coefficients[0] = (1 + V/q * K + K * K) * norm;
      coefficients[1] = 2 * (K * K - 1) * norm;
      coefficients[2] = (1 - V/q * K + K * K) * norm;
      coefficients[3] = - coefficients[1];
      coefficients[4] = - (1 - 1/q * K + K * K) * norm;
    }
    else {
      norm = 1 / (1 + V/q * K + K * K);
      coefficients[0] = (1 + 1/q * K + K * K) * norm;
      coefficients[1] = 2 * (K * K - 1) * norm;
      coefficients[2] = (1 - 1/q * K + K * K) * norm;
      coefficients[3] = - coefficients[1];
      coefficients[4] = - (1 - V/q * K + K * K) * norm;
    }
  }

  static void setLowShelf(float* coefficients, float fc, float gain){
    float omega = M_PI*fc/2;
    float K = tanf(omega);
    float V = abs(gain-0.5)*60 + 1; // Gain
    float norm;
    if(gain >= 0.5) {
      norm = 1 / (1 + M_SQRT2 * K + K * K);
      coefficients[0] = (1 + sqrtf(2*V) * K + V * K * K) * norm;
      coefficients[1] = 2 * (V * K * K - 1) * norm;
      coefficients[2] = (1 - sqrtf(2*V) * K + V * K * K) * norm;
      coefficients[3] = - 2 * (K * K - 1) * norm;
      coefficients[4] = - (1 - M_SQRT2 * K + K * K) * norm;
    } else {
      norm = 1 / (1 + sqrtf(2*V) * K + V * K * K);
      coefficients[0] = (1 + M_SQRT2 * K + K * K) * norm;
      coefficients[1] = 2 * (K * K - 1) * norm;
      coefficients[2] = (1 - M_SQRT2 * K + K * K) * norm;
      coefficients[3] = - 2 * (V * K * K - 1) * norm;
      coefficients[4] = - (1 - sqrtf(2*V) * K + V * K * K) * norm;
    }
  }

  static void setHighShelf(float* coefficients, float fc, float gain){
    float omega = M_PI*fc/2;
    float K = tanf(omega);
    float V = abs(gain-0.5)*60 + 1; // Gain
    float norm;
    if(gain >= 0.5) {
      norm = 1 / (1 + M_SQRT2 * K + K * K);
      coefficients[0] = (V + sqrtf(2*V) * K + K * K) * norm;
      coefficients[1] = 2 * (K * K - V) * norm;
      coefficients[2] = (V - sqrtf(2*V) * K + K * K) * norm;
      coefficients[3] = - 2 * (K * K - 1) * norm;
      coefficients[4] = - (1 - M_SQRT2 * K + K * K) * norm;
    } else {
      norm = 1 / (V + sqrtf(2*V) * K + K * K);
      coefficients[0] = (1 + M_SQRT2 * K + K * K) * norm;
      coefficients[1] = 2 * (K * K - 1) * norm;
      coefficients[2] = (1 - M_SQRT2 * K + K * K) * norm;
      coefficients[3] = - 2 * (K * K - V) * norm;
      coefficients[4] = - (V - sqrtf(2*V) * K + K * K) * norm;
    }
  }

};

/** 
 * Cascaded Biquad Filter.
 * Implemented using CMSIS DSP Library, Direct Form 2 Transposed.
 * Each cascaded stage implements a second order filter.
 */
#define BIQUAD_COEFFICIENTS_PER_STAGE    5
#define BIQUAD_STATE_VARIABLES_PER_STAGE 2
class BiquadFilter {
private:
#ifdef ARM_CORTEX
  // arm_biquad_casd_df1_inst_f32 df1;
  arm_biquad_cascade_df2T_instance_f32 df2;
#endif /* ARM_CORTEX */
protected:
  float* coefficients; // stages*5
  float* state; // stages*4 for df1, stages*2 for df2
  int stages;
  /*
   * The coefficients are stored in the array <code>coefficients</code> in the following order:
   * <pre>
   *     {b10, b11, b12, a11, a12, b20, b21, b22, a21, a22, ...}
   * </pre>
   * where <code>b1x</code> and <code>a1x</code> are the coefficients for the first stage,
   * <code>b2x</code> and <code>a2x</code> are the coefficients for the second stage,
   * and so on.  The <code>coeffs</code> array must contain a total of <code>5*stages</code> values.   
   */
  void copyCoefficients(){
    for(int i=1; i<stages; ++i){
      coefficients[0+i*5] = coefficients[0];
      coefficients[1+i*5] = coefficients[1];
      coefficients[2+i*5] = coefficients[2];
      coefficients[3+i*5] = coefficients[3];
      coefficients[4+i*5] = coefficients[4];
    }
  }
  void init(){
#ifdef ARM_CORTEX
    // arm_biquad_cascade_df1_init_f32(&df1, stages, coefficients, state);
    arm_biquad_cascade_df2T_init_f32(&df2, stages, coefficients, state);
#else
    for(int n=0; n<stages*BIQUAD_STATE_VARIABLES_PER_STAGE; n++){
      state[n]=0;
    }
#endif /* ARM_CORTEX */
  }
public:
  BiquadFilter()
    : coefficients(NULL), state(NULL), stages(0) {}

  BiquadFilter(float* coefs, float* ste, int sgs) :
    coefficients(coefs), state(ste), stages(sgs) {
    init();
  }

  int getStages(){
    return stages;
  }

  static int getCoefficientsPerStage(){
    return BIQUAD_COEFFICIENTS_PER_STAGE;
  }

  FloatArray getCoefficients(){
    return FloatArray(coefficients, BIQUAD_COEFFICIENTS_PER_STAGE*stages);
  }

  FloatArray getState(){
    return FloatArray(state, BIQUAD_STATE_VARIABLES_PER_STAGE*stages);
  }

  FilterStage getFilterStage(int stage){
    ASSERT(stage < stages, "Invalid filter stage index");
    FloatArray c(coefficients+BIQUAD_COEFFICIENTS_PER_STAGE*stage, BIQUAD_COEFFICIENTS_PER_STAGE);
    FloatArray s(state+BIQUAD_STATE_VARIABLES_PER_STAGE*stage, BIQUAD_STATE_VARIABLES_PER_STAGE);
    return FilterStage(c, s);
  }

  /* process into output, leaving input intact */
  void process(float* input, float* output, int size){
#ifdef ARM_CORTEX
    // arm_biquad_cascade_df1_f32(&df1, input, output, size);
    arm_biquad_cascade_df2T_f32(&df2, input, output, size);
#else
    for(int k=0; k<stages; k++){
      float b0=getFilterStage(k).getCoefficients()[0];
      float b1=getFilterStage(k).getCoefficients()[1];
      float b2=getFilterStage(k).getCoefficients()[2];
      float a1=getFilterStage(k).getCoefficients()[3];
      float a2=getFilterStage(k).getCoefficients()[4];
      float d1=state[k*BIQUAD_STATE_VARIABLES_PER_STAGE];
      float d2=state[k*BIQUAD_STATE_VARIABLES_PER_STAGE+1];
      for(int n=0; n<size; n++){ //manually apply filter, one stage
        float out=b0 * input[n] + d1; 
        d1 = b1 * input[n] + a1 * out + d2;
        d2 = b2 * input[n] + a2 * out;
        output[n]=out;
        state[k*BIQUAD_STATE_VARIABLES_PER_STAGE]=d1;
        state[k*BIQUAD_STATE_VARIABLES_PER_STAGE+1]=d2;
      }
    }
#endif /* ARM_CORTEX */
  }

  /* perform in-place processing */
  void process(float* buf, int size){
    process(buf, buf, size);
  }

  void process(FloatArray in){
    process(in, in, in.getSize());
  }

  void process(FloatArray in, FloatArray out){
    ASSERT(out.getSize() >= in.getSize(), "output array must be at least as long as input");
    process(in, out, in.getSize());
  }

  /* process a single sample and return the result */
  float process(float input){
    float output;
    process(&input, &output, 1);
    return output;
  }

  void setLowPass(float fc, float q){
    FilterStage::setLowPass(coefficients, fc, q);
    copyCoefficients();
  }

  void setHighPass(float fc, float q){
    FilterStage::setHighPass(coefficients, fc, q);
    copyCoefficients();
  }

  void setBandPass(float fc, float q){
    FilterStage::setBandPass(coefficients, fc, q);
    copyCoefficients();
  }
  void setNotch(float fc, float q){
    FilterStage::setNotch(coefficients, fc, q);
    copyCoefficients();
  }
  void setPeak(float fc, float q, float gain){
    FilterStage::setPeak(coefficients, fc, q, gain);
    copyCoefficients();
  }
  void setLowShelf(float fc, float gain){
    FilterStage::setLowShelf(coefficients, fc, gain);
    copyCoefficients();
  }
  void setHighShelf(float fc, float gain){
    FilterStage::setHighShelf(coefficients, fc, gain);
    copyCoefficients();
  }

  void setCoefficientsPointer(FloatArray newCoefficients){ //sets coefficients to point to a given pointer
    ASSERT(BIQUAD_COEFFICIENTS_PER_STAGE*stages==newCoefficients.getSize(), "wrong size");
    coefficients = newCoefficients;
    init();
  }

  void setCoefficients(FloatArray newCoefficients){//copies coefficients to all stages
    ASSERT(newCoefficients.getSize()==BIQUAD_COEFFICIENTS_PER_STAGE, "wrong size");
    getFilterStage(0).setCoefficients(newCoefficients);
    copyCoefficients(); //set all the other stages
  }

  static BiquadFilter* create(int stages){
    return new BiquadFilter(new float[stages*5], new float[stages*2], stages);
    // for df1: state requires stages*4
    // return new BiquadFilter(new float[stages*5], new float[stages*4], stages);
  }

  static void destroy(BiquadFilter* filter){
    delete filter->coefficients;
    delete filter->state;
    delete filter;
  }
};

class StereoBiquadFilter : public BiquadFilter {
private:
  BiquadFilter right;
public:
  StereoBiquadFilter(float* coefs, float* lstate, float* rstate, int sgs) : 
    BiquadFilter(coefs, lstate, sgs), 
    right(coefs, rstate, sgs) {}

  BiquadFilter* getLeftFilter(){
    return this;
  }

  BiquadFilter* getRightFilter(){
    return &right;
  }

  void process(AudioBuffer &buffer){
    BiquadFilter::process(buffer.getSamples(LEFT_CHANNEL));
    right.process(buffer.getSamples(RIGHT_CHANNEL));
  }

  static StereoBiquadFilter* create(int stages){
    return new StereoBiquadFilter(new float[stages*5], new float[stages*2], new float[stages*2], stages);
  }

  static void destroy(StereoBiquadFilter* filter){
    FloatArray::destroy(filter->right.getState());
    BiquadFilter::destroy(filter);
  }
};

const float FilterStage::BESSEL_Q = 1/sqrtf(3); // 1/sqrt(3)
const float FilterStage::SALLEN_KEY_Q = 0.5f; // 1/2
const float FilterStage::BUTTERWORTH_Q = 1/sqrtf(2); // 1/sqrt(2)

#endif // __BiquadFilter_h__
