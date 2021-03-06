// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include "kudu/client/scan_predicate.h"

#include <boost/optional.hpp>
#include <utility>

#include "kudu/client/scan_predicate-internal.h"
#include "kudu/client/value-internal.h"
#include "kudu/client/value.h"
#include "kudu/common/scan_predicate.h"
#include "kudu/common/scan_spec.h"
#include "kudu/gutil/strings/substitute.h"

using std::move;
using boost::optional;

namespace kudu {

using strings::Substitute;

namespace client {

KuduPredicate::KuduPredicate(Data* d)
  : data_(d) {
}

KuduPredicate::~KuduPredicate() {
  delete data_;
}

KuduPredicate::Data::Data() {
}

KuduPredicate::Data::~Data() {
}

KuduPredicate* KuduPredicate::Clone() const {
  return new KuduPredicate(data_->Clone());
}

ComparisonPredicateData::ComparisonPredicateData(ColumnSchema col,
                                                 KuduPredicate::ComparisonOp op,
                                                 KuduValue* val)
    : col_(move(col)),
      op_(op),
      val_(val) {
}
ComparisonPredicateData::~ComparisonPredicateData() {
}


Status ComparisonPredicateData::AddToScanSpec(ScanSpec* spec, Arena* arena) {
  void* val_void;
  RETURN_NOT_OK(val_->data_->CheckTypeAndGetPointer(col_.name(),
                                                    col_.type_info()->physical_type(),
                                                    &val_void));
  switch (op_) {
    case KuduPredicate::LESS_EQUAL: {
      optional<ColumnPredicate> pred =
        ColumnPredicate::InclusiveRange(col_, nullptr, val_void, arena);
      if (pred) {
        spec->AddPredicate(*pred);
      }
      break;
    };
    case KuduPredicate::GREATER_EQUAL: {
      spec->AddPredicate(ColumnPredicate::Range(col_, val_void, nullptr));
      break;
    };
    case KuduPredicate::EQUAL: {
      spec->AddPredicate(ColumnPredicate::Equality(col_, val_void));
      break;
    };
    default:
      return Status::InvalidArgument(Substitute("invalid comparison op: $0", op_));
  }
  return Status::OK();
}

} // namespace client
} // namespace kudu
