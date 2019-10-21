/*
 * Copyright 2018 Google
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FIRESTORE_CORE_SRC_FIREBASE_FIRESTORE_NANOPB_READER_H_
#define FIRESTORE_CORE_SRC_FIREBASE_FIRESTORE_NANOPB_READER_H_

#include <pb.h>
#include <pb_decode.h>

#include <cstdint>
#include <utility>
#include <vector>

#include "Firestore/core/include/firebase/firestore/firestore_errors.h"
#include "Firestore/core/src/firebase/firestore/nanopb/byte_string.h"
#include "Firestore/core/src/firebase/firestore/util/status.h"
#include "absl/strings/string_view.h"
#include "grpcpp/support/byte_buffer.h"

namespace firebase {
namespace firestore {
namespace nanopb {

class ReadContext {
 public:
  bool ok() const {
    return status_.ok();
  }

  const util::Status& status() const {
    return status_;
  }

  void set_status(util::Status status) {
    status_ = std::move(status);
  }

  /**
   * Ensures this Reader's status is `!ok().
   *
   * If this Reader's status is already !ok(), then this may augment the
   * description, but will otherwise leave it alone. Otherwise, this Reader's
   * status will be set to Error::DataLoss with the specified
   * description.
   */
  void Fail(absl::string_view description) {
    status_.Update(util::Status(Error::DataLoss, description));
  }

 private:
  util::Status status_ = util::Status::OK();
};

/**
 * Docs TODO(rsgowman). But currently, this just wraps the underlying nanopb
 * pb_istream_t.
 */
class Reader {
 public:
  virtual ~Reader() = default;

  /**
   * Reads a Nanopb proto from the input stream.
   *
   * This essentially wraps calls to nanopb's pb_decode() method. This is the
   * primary way of decoding messages.
   *
   * Note that this allocates memory. You must call nanopb::FreeNanopbMessage()
   * (which essentially wraps pb_release()) on the dest_struct in order to avoid
   * memory leaks. (This also implies code that uses this is not exception
   * safe.)
   */
  // TODO(rsgowman): At the moment we rely on the caller to manually free
  // dest_struct via nanopb::FreeNanopbMessage(). We might instead see if we can
  // register allocated messages, track them, and free them ourselves. This may
  // be especially relevant if we start to use nanopb messages as the underlying
  // data within the model objects.
  virtual void Read(const pb_field_t fields[], void* dest_struct) = 0;

  bool ok() const {
    return context_.ok();
  }

  const util::Status& status() const {
    return context_.status();
  }

  void set_status(util::Status status) {
    context_.set_status(std::move(status));
  }

  ReadContext* context() {
    return &context_;
  }

  const ReadContext* context() const {
    return &context_;
  }

  /**
   * Ensures this Reader's status is `!ok().
   *
   * If this Reader's status is already !ok(), then this may augment the
   * description, but will otherwise leave it alone. Otherwise, this Reader's
   * status will be set to Error::DataLoss with the specified
   * description.
   */
  void Fail(absl::string_view description) {
    context_.Fail(description);
  }

 private:
  ReadContext context_;
};

class StringReader : public Reader {
 public:
  /**
   * Creates an instance that isn't associated with any bytes. It can be used
   * for error propagation.
   * TODO(varconst): only use `ReadContext` for error propagation.
   */
  StringReader() = default;

  /**
   * Creates an input stream that reads from the specified bytes. Note that
   * this reference must remain valid for the lifetime of this `StringReader`.
   *
   * (This is roughly equivalent to the Nanopb function
   * `pb_istream_from_buffer()`)
   *
   * @param bytes where the input should be deserialized from.
   */
  explicit StringReader(const nanopb::ByteString& bytes);
  explicit StringReader(const std::vector<uint8_t>& bytes);
  StringReader(const uint8_t* bytes, size_t length);

  /**
   * Creates an input stream from bytes backing the string_view. Note that
   * the backing buffer must remain valid for the lifetime of this
   * `StringReader`.
   *
   * (This is roughly equivalent to the Nanopb function
   * `pb_istream_from_buffer()`)
   */
  explicit StringReader(absl::string_view);

  void Read(const pb_field_t fields[], void* dest_struct) override;

 private:
  /**
   * Takes that a shallow copy of the given `stream`. (Non-null pointers within
   * this struct must remain valid for the lifetime of this `StringReader`.)
   */
  explicit StringReader(pb_istream_t stream) : stream_(stream) {
  }

  pb_istream_t stream_{};
};

class ByteBufferReader : public Reader {
 public:
  explicit ByteBufferReader(const grpc::ByteBuffer& buffer);

  void Read(const pb_field_t* fields, void* dest_struct) override;

 private:
  ByteString bytes_;
  pb_istream_t stream_{};
};

}  // namespace nanopb
}  // namespace firestore
}  // namespace firebase

#endif  // FIRESTORE_CORE_SRC_FIREBASE_FIRESTORE_NANOPB_READER_H_
