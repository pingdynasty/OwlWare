#ifndef __FloatArray_h__
#define __FloatArray_h__

#include <cstddef>

#ifndef ASSERT
#include "owlcontrol.h"
//#define ASSERT(cond, msg) do{if(!(cond))setErrorMessage(PROGRAM_ERROR, msg);}while(0)
//#define ASSERT(cond, msg)
#endif

/**
 * This class contains useful methods for manipulating arrays of floats.
 * It also provides a convenient handle to the array pointer and the size of the array.
 * FloatArray objects can be passed by value without copying the contents of the array.
 */
class FloatArray {
private:
  float* data;
  int size;
public:
  FloatArray();
  FloatArray(float* data, int size);

  int getSize() const{
    return size;
  }

  int getSize(){
    return size;
  }

  /**
   * Clear the array.
   * Set all the values in the array to 0.
  */
  void clear(){
    setAll(0);
  }
  
  /**
   * Get the minimum value in the array and its index
   * @param[out] value will be set to the minimum value after the call
   * @param[out] index will be set to the index of the minimum value after the call
   * 
   */
  void getMin(float* value, int* index);
  
  /**
   * Get the maximum value in the array and its index
   * @param[out] value will be set to the maximum value after the call
   * @param[out] index will be set to the index of the maximum value after the call
  */
  void getMax(float* value, int* index);
  
  /**
   * Get the minimum value in the array
   * @return the minimum value contained in the array
  */
  float getMinValue();
  
  /**
   * Get the maximum value in the array
   * @return the maximum value contained in the array
   */
  float getMaxValue();
  
  /**
   * Get the index of the minimum value in the array
   * @return the mimimum value contained in the array
   */
  int getMinIndex();
  
  /**
   * Get the index of the maximum value in the array
   * @return the maximum value contained in the array
   */
  int getMaxIndex();
  
  float getDb(); //TODO: not implemented
  
  /**
   * Absolute value of the array.
   * Stores the absolute value of the elements in the array into destination.
   * @param[out] destination the destination array.
  */
  void rectify(FloatArray& destination);
  
  /**
   * Absolute value of the array.
   * Sets each element in the array to its absolute value.
  */
  void rectify(); //in place
  
  /**
   * Reverse the array
   * Copies the elements of the array in reversed order into destination.
   * @param[out] destination the destination array.
  */
  void reverse(FloatArray& destination);
  
  /**
   * Reverse the array.
   * Reverses the order of the elements in the array.
  */
  void reverse(); //in place
  
  /**
   * Reciprocal of the array.
   * Stores the reciprocal of the elements in the array into destination.
   * @param[out] destination the destination array.
  */
  void reciprocal(FloatArray& destination);
  
  /**
   * Reciprocal of the array.
   * Sets each element in the array to its reciprocal.
  */
  void reciprocal(); 
  
  /**
   * Negate the array.
   * Stores the opposite of the elements in the array into destination.
   * @param[out] destination the destination array.
  */
  void negate(FloatArray& destination);
  
  /**
   * Copy and negate the array.
   * Sets each element in the array to its opposite.
  */
  void negate(); 
  
  /**
   * Random values
   * Fills the array with random values in the range [-1, 1)
   */
  void noise();
  
  /**
   * Random values in range.
   * Fills the array with random values in the range [**min**, **max**)
   * @param min minimum value in the range
   * @param max maximum value in the range 
   */
  void noise(float min, float max);
  
  /**
   * Root mean square value of the array.
   * Gets the root mean square of the values in the array.
  */
  float getRms();
  
  /**
   * Mean of the array.
   * Gets the mean of the values in the array.
  */
  float getMean();
  
  /**
   * Power of the array.
   * Gets the power of the values in the array.
  */
  float getPower();
  
  /**
   * Standard deviation of the array.
   * Gets the standard deviation of the values in the array.
  */
  float getStandardDeviation();
  
  /**
   * Variance of the array.
   * Gets the variance of the values in the array.
  */
  float getVariance();
  
  /**
   * Array by scalar multiplication.
   * Array by scalar multiplication, same as multiply(float).
   * @param[out] destination the destination array
   * @param[in] factor the scaling factor
  */
  void scale(float factor, FloatArray destination);
  
  /**
   * Array by scalar multiplication, in-place.
   * @param[in] factor the scaling factor
  */
  void scale(float factor);
  
  /**
   * Clips the elements in the array in the range [-1, 1].
  */
  void clip();
  
  /**
   * Clips the elements in the array in the range [-**range**, **range**].
   * @param range clipping value.
  */
  void clip(float range);
  
  /**
   * Clips the elements in the array in the range [**min**, **max**].
   * @param min minimum value
   * @param max maximum value
  */
  void clip(float min, float max);
  
  /**
   * Element-wise sum between arrays.
   * Sets each element in **destination** to the sum of the corresponding element of the array and **operand2**
   * @param[in] operand2 second operand for the sum
   * @param[out] destination the destination array
  */
  void add(FloatArray operand2, FloatArray destination);
  
  /**
   * Element-wise sum between arrays.
   * Adds each element of **operand2** to the corresponding element in the array.
   * @param operand2 second operand for the sum
  */
  void add(FloatArray operand2); //in-place
  
  /**
   * Array-scalar sum.
   * Adds **scalar** to the values in the array.
   * @param scalar value to be added to the array
  */
  void add(float scalar);
  
  /**
   * Element-wise difference between arrays.
   * Sets each element in **destination** to the difference between the corresponding element of the array and **operand2**
   * @param[in] operand2 second operand for the subtraction
   * @param[out] destination the destination array
  */
  void subtract(FloatArray operand2, FloatArray destination);
  
  
  /**
   * Element-wise difference between arrays.
   * Subtracts from each element of the array the corresponding element in **operand2**.
   * @param[in] operand2 second operand for the subtraction
  */
  void subtract(FloatArray operand2); //in-place
  
  /**
   * Array-scalar subtraction.
   * Subtracts **scalar** from the values in the array.
   * @param scalar to be subtracted from the array
  */
  void subtract(float scalar);
  
/**
   * Element-wise multiplication between arrays.
   * Sets each element in **destination** to the product of the corresponding element of the array and **operand2**
   * @param[in] operand2 second operand for the product
   * @param[out] destination the destination array
  */
  void multiply(FloatArray operand2, FloatArray destination);
  
   /**
   * Element-wise multiplication between arrays.
   * Multiplies each element in the array by the corresponding element in **operand2**.
   * @param operand2 second operand for the sum
  */
  void multiply(FloatArray operand2); //in-place
  
  /**
   * Array-scalar multiplication.
   * Multiplies the values in the array by **scalar**.
   * @param scalar to be subtracted from the array
  */
  void multiply(float scalar);
  
  /**
   * Convolution between arrays.
   * Sets **destination** to the result of the convolution between the array and **operand2**
   * @param[in] operand2 the second operand for the convolution
   * @param[out] destination array. It must have a minimum size of this+other-1.
  */
  void convolve(FloatArray operand2, FloatArray destination);
  
  /** 
   * Partial convolution between arrays.
   * Perform partial convolution: start at **offset** and compute **samples** values.
   * @param[in] operand2 the second operand for the convolution.
   * @param[out] destination the destination array.
   * @param[in] offset first output sample to compute
   * @param[in] samples number of samples to compute
   * @remarks **destination[n]** is left unchanged for n<offset and the result is stored from destination[offset] onwards
   * that is, in the same position where they would be if a full convolution was performed.
  */
  void convolve(FloatArray operand2, FloatArray destination, int offset, int samples);
  
  /** 
   * Correlation between arrays.
   * Sets **destination** to the correlation of the array and **operand2**.
   * @param[in] operand2 the second operand for the correlation
   * @param[out] destination the destination array. It must have a minimum size of 2*max(srcALen, srcBLen)-1
  */
  void correlate(FloatArray operand2, FloatArray destination);
  
  /**
   * Correlation between arrays.
   * Sets **destination** to the correlation of *this* array and **operand2**.
   * @param[in] operand2 the second operand for the correlation
   * @param[out] destination array. It must have a minimum size of 2*max(srcALen, srcBLen)-1
   * @remarks It is the same as correlate(), but destination must have been initialized to 0 in advance. 
  */
  void correlateInitialized(FloatArray operand2, FloatArray destination);

  /**
   * Set all the values in the array.
   * Sets all the elements of the array to **value**.
   * @param[in] value all the elements are set to this value.
  */
  void setAll(float value);
  
  /**
   * A subset of the array.
   * Returns a array that points to subset of the memory used by the original array.
   * @param[in] offset the first element of the subset.
   * @param[in] length the number of elments in the new FloatArray.
   * @return the newly created FloatArray.
   * @remarks no memory is allocated by this method. The memory is still shared with the original array.
   * The memory should not be de-allocated elsewhere (e.g.: by calling FloatArray::destroy() on the original FloatArray) 
   * as long as the FloatArray returned by this method is still in use.
   * @remarks Calling FloatArray::destroy() on a FloatArray instance created with this method might cause an exception.
  */
  FloatArray subArray(int offset, int length);
  
  /**
   * Copies the content of the array to another array.
   * @param[out] destination the destination array
  */
  void copyTo(FloatArray destination);

  /**
   * Copies the content of the array to a location in memory.
   * @param[out] destination a pointer to the beginning of the memory location to copy to.
   * The **length***sizeof(float) bytes of memory starting at this location must have been allocated before calling this method.
   * @param[in] length number of samples to copy
  */
  void copyTo(float* destination, int length);

  /**
   * Copies the content of an array into another array.
   * @param[in] source the source array
  */
  void copyFrom(FloatArray source);
  
  /**
   * Copies an array of float into the array.
   * @param[in] source a pointer to the beginning of the portion of memory to read from.
   * @param[in] length number of samples to copy.
  */
  void copyFrom(float* source, int length);
  
  /**
   * Copies the content of an array into a subset of the array.
   * Copies **samples** elements from **source** to **destinationOffset** in the current array.
   * @param[in] source the source array
   * @param[in] destinationOffset the offset into the destination array 
   * @param[in] samples the number of samples to copy
   *
  */
  void insert(FloatArray source, int destinationOffset, int samples);

  /**
   * Copies the content of an array into a subset of the array.
   * Copies **samples** elements starting from **sourceOffset** of **source** to **destinationOffset** in the current array.
   * @param[in] source the source array
   * @param[in] sourceOffset the offset into the source array
   * @param[in] destinationOffset the offset into the destination array
   * @param[in] samples the number of samples to copy
  */
  void insert(FloatArray source, int sourceOffset, int destinationOffset, int samples);
  
  /**
   * Copies values within an array.
   * Copies **length** values starting from index **fromIndex** to locations starting with index **toIndex**
   * @param[in] fromIndex the first element to copy
   * @param[in] toIndex the destination of the first element
   * @param[in] length the number of elements to copy
   * @remarks this method uses *memmove()* so that the source memory and the destination memory can overlap. As a consequence it might have slow performances.
  */
  void move(int fromIndex, int toIndex, int length);
  
  /**
   * Allows to index the array using array-style brackets.
   * @param index the index of the element
   * @return the value of the **index** element of the array
   * Example usage:
   * @code
   * int size=1000;
   * float content[size]; 
   * FloatArray floatArray(content, size);
   * for(int n=0; n<size; n++)
   *   content[n]==floatArray[n]; //now the FloatArray can be indexed as if it was an array
   * @endcode
  */
  float& operator [](const int index){
    return data[index];
  }
  
  /**
   * Allows to index the array using array-style brackets.
   * **const** version of operator[]
  */
  float& operator [](const int index) const{
    return data[index];
  }
  
  /**
   * Compares two arrays.
   * Performs an element-wise comparison of the values contained in the arrays.
   * @param other the array to compare against.
   * @return **true** if the arrays have the same size and the value of each of the elements of the one 
   * match the value of the corresponding element of the other, or **false** otherwise.
  */
  bool equals(const FloatArray& other) const{
    if(size!=other.getSize()){
      return false;
    }
    for(int n=0; n<size; n++){
      if(data[n]!=other[n]){
        return false;
      }
    }
    return true;
  }
  
  /**
   * Casting operator to float*
   * @return a float* pointer to the data stored in the FloatArray
  */
  operator float*(){
    return data;
  }
  
  /**
   * Get the data stored in the FloatArray.
   * @return a float* pointer to the data stored in the FloatArray
  */
  float* getData(){
    return data;
  }
  
  /**
   * Creates a new FloatArray.
   * Allocates size*sizeof(float) bytes of memory and returns a FloatArray that points to it.
   * @param size the size of the new FloatArray.
   * @return a FloatArray which **data** point to the newly allocated memory and **size** is initialized to the proper value.
   * @remarks a FloatArray created with this method has to be destroyed invoking the FloatArray::destroy() method.
  */
  static FloatArray create(int size);
  
  /**
   * Destroys a FloatArray created with the create() method.
   * @param array the FloatArray to be destroyed.
   * @remarks the FloatArray object passed as an argument should not be used again after invoking this method.
   * @remarks a FloatArray object that has not been created by the FloatArray::create() method might cause an exception if passed as an argument to this method.
  */
  static void destroy(FloatArray array);
};

#endif // __FloatArray_h__
