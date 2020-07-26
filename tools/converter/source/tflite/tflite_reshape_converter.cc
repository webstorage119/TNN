// Tencent is pleased to support the open source community by making TNN available.
//
// Copyright (C) 2020 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// https://opensource.org/licenses/BSD-3-Clause
//
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

#include "tflite_op_converter.h"

namespace TNN_CONVERTER {
DECLARE_OP_CONVERTER(Reshape);

std::string TFLiteReshapeConverter::TNNOpType(bool quantizedModel) {
    if (quantizedModel) {
        return "QuantizedReshape";
    }
    return "Reshape";
}

TNN_NS::Status TFLiteReshapeConverter::exec(TNN_NS::NetStructure& net_structure, TNN_NS::NetResource& net_resource,
                                            const std::unique_ptr<tflite::OperatorT>& tf_lite_operator,
                                            const std::vector<std::unique_ptr<tflite::TensorT>>& tf_lite_tensors,
                                            const std::vector<std::unique_ptr<tflite::BufferT>>& tf_lite_model_buffer,
                                            const std::vector<std::unique_ptr<tflite::OperatorCodeT>>& tf_lite_op_set,
                                            bool quantizedModel) {
    TNN_NS::ReshapeLayerParam* param = new TNN_NS::ReshapeLayerParam;
    auto cur_layer                   = net_structure.layers.back();
    auto tf_lite_op_type             = tf_lite_op_set[tf_lite_operator->opcode_index]->builtin_code;
    const auto& reshape_option       = tf_lite_operator->builtin_options.AsReshapeOptions();

    if (quantizedModel) {
        // TODO
    } else {
        param->name      = cur_layer->name;
        param->type      = cur_layer->type_str;
        param->quantized = false;
        param->axis      = 0;
        param->num_axes  = 4;

        const auto& shape_tensor = tf_lite_tensors[tf_lite_operator->inputs[1]];
        ASSERT(shape_tensor->type == tflite::TensorType_INT32);

        int shape_size = 1;
        for (int i = 0; i < shape_tensor->shape.size(); ++i) {
            shape_size *= shape_tensor->shape[i];
        }

        const auto& shape_data = tf_lite_model_buffer[shape_tensor->buffer]->data;
        ASSERT(shape_size == shape_data.size() / 4);

        auto dimPtr = reinterpret_cast<const int32_t*>(shape_data.data());
        std::vector<int> reshape_dim(dimPtr, dimPtr + shape_size);
    }

    // set input output index
    cur_layer->inputs.resize(1);
    cur_layer->outputs.resize(1);
    cur_layer->inputs[0]  = tf_lite_tensors[tf_lite_operator->inputs[0]]->name;
    cur_layer->outputs[0] = tf_lite_tensors[tf_lite_operator->outputs[0]]->name;
    return TNN_NS::TNN_CONVERT_OK;
}

using namespace tflite;
REGISTER_CONVERTER(Reshape, BuiltinOperator_RESHAPE);
}  // namespace TNN_CONVERTER