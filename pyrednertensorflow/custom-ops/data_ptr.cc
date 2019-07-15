/* Copyright 2019 Seyoung Park. All Rights Reserved.

==============================================================================*/

#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/framework/shape_inference.h"
#include "tensorflow/core/framework/op_kernel.h"
#include <stdint.h>
#include <climits>

using namespace tensorflow;

/* Tensorflow custom ops does not allow parameter types of list of 
   various data types. Therefore, we can't pass a list but we have
   to pass each objects individually. 

   Consult Tensorflow source code: /tensorflow/core/framework/tensor.h
   for what is supported by Tensorflow
*/

REGISTER_OP("DataPtr")
    .Attr("T: {float, int32} = DT_INT32")  // To preserve backwards compatibility, you should specify a default value when adding an attr to an existing op:
    .Input("input: T")  // Tensor
    .Output("output: uint64")  // scalar
    .SetShapeFn([](::tensorflow::shape_inference::InferenceContext* c) {
      c->set_output(0, {}); // scalar
      return Status::OK();
    });


template <typename T>
class DataPtrOp : public OpKernel {
 public:
  explicit DataPtrOp(OpKernelConstruction* context) : OpKernel(context) {}

  void Compute(OpKernelContext* context) override {
    // Grab the input tensor

    const Tensor& input_tensor = context->input(0);
    auto tensor = input_tensor.flat<T>();
    // const Tensor& input_tensor = context->mutable_input(0);
    // ofstream("tensor_out_ops.txt") << tensor[0] << " " << tensor[1] << endl;

    // Create an output tensor
    // NOTE: The output datatype must match the Ops definition!!!.
    Tensor* output_tensor = NULL;
    OP_REQUIRES_OK(context, 
      context->allocate_output(0, {},  // Initialize a one-element scalar
      &output_tensor)
      );
    auto output_flat = output_tensor->flat<uint64>();

    // Cast pointer to unsigned long int     
    uintptr_t addr = (uintptr_t)&tensor(0);//reinterpret_cast<uintptr_t>(&input_tensor[0]);

    // Cast unsigned long int -> unsigned int64
    uint64 addr_converted = addr;

    output_flat(0) = addr_converted;

  }
};

// Polymorphism: https://www.tensorflow.org/guide/extend/op#polymorphism
REGISTER_KERNEL_BUILDER(
  Name("DataPtr")
  .Device(DEVICE_CPU)
  .TypeConstraint<int32>("T"),
  DataPtrOp<int32>);
REGISTER_KERNEL_BUILDER(
  Name("DataPtr")
  .Device(DEVICE_CPU)
  .TypeConstraint<float>("T"),
  DataPtrOp<float>);
  
