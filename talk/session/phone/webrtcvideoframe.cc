/*
 * libjingle
 * Copyright 2004--2011, Google Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "talk/session/phone/webrtcvideoframe.h"

#include "talk/base/logging.h"
#include "talk/session/phone/videocapturer.h"
#include "talk/session/phone/videocommon.h"
#ifdef WEBRTC_RELATIVE_PATH
#include "common_video/vplib/main/interface/vplib.h"
#else
#include "third_party/webrtc/files/include/vplib.h"
#endif

namespace cricket {

WebRtcVideoFrame::WebRtcVideoFrame() {
}

WebRtcVideoFrame::~WebRtcVideoFrame() {
}

bool WebRtcVideoFrame::Init(uint32 format, int w, int h, int dw, int dh,
                            uint8* sample, size_t sample_size,
                            size_t pixel_width, size_t pixel_height,
                            int64 elapsed_time, int64 time_stamp,
                            int rotation) {
  // WebRtcVideoFrame currently doesn't support color conversion or rotation.
  if (format != FOURCC_I420 || dw != w || dh != h || rotation != 0) {
    return false;
  }

  uint8* buffer = new uint8[sample_size];
  memcpy(buffer, sample, sample_size);
  Attach(buffer, sample_size, w, h, pixel_width, pixel_height,
         elapsed_time, time_stamp, rotation);
  return true;
}

bool WebRtcVideoFrame::Init(const CapturedFrame* frame, int dw, int dh) {
  return Init(frame->fourcc, frame->width, frame->height, dw, dh,
              static_cast<uint8*>(frame->data), frame->data_size,
              frame->pixel_width, frame->pixel_height,
              frame->elapsed_time, frame->time_stamp, frame->rotation);
}

bool WebRtcVideoFrame::InitToBlack(int w, int h,
                                   size_t pixel_width, size_t pixel_height,
                                   int64 elapsed_time, int64 time_stamp) {
  size_t buffer_size = VideoFrame::SizeOf(w, h);
  uint8* buffer = new uint8[buffer_size];
  Attach(buffer, buffer_size, w, h, pixel_width, pixel_height,
         elapsed_time, time_stamp, 0);
  memset(GetYPlane(), 16, w * h);
  memset(GetUPlane(), 128, w * h / 4);
  memset(GetVPlane(), 128, w * h / 4);
  return true;
}

void WebRtcVideoFrame::Attach(uint8* buffer, size_t buffer_size, int w, int h,
                              size_t pixel_width, size_t pixel_height,
                              int64 elapsed_time, int64 time_stamp,
                              int rotation) {
  video_frame_.Free();
  WebRtc_UWord8* new_memory = buffer;
  WebRtc_UWord32 new_length = buffer_size;
  WebRtc_UWord32 new_size = buffer_size;
  video_frame_.Swap(new_memory, new_length, new_size);
  video_frame_.SetWidth(w);
  video_frame_.SetHeight(h);
  pixel_width_ = pixel_width;
  pixel_height_ = pixel_height;
  elapsed_time_ = elapsed_time;
  video_frame_.SetTimeStamp(static_cast<WebRtc_UWord32>(time_stamp));
  rotation_ = rotation;
}

void WebRtcVideoFrame::Detach(uint8** buffer, size_t* buffer_size) {
  WebRtc_UWord8* new_memory = NULL;
  WebRtc_UWord32 new_length = 0;
  WebRtc_UWord32 new_size = 0;
  video_frame_.Swap(new_memory, new_length, new_size);
  *buffer = new_memory;
  *buffer_size = new_size;
}

size_t WebRtcVideoFrame::GetWidth() const {
  return video_frame_.Width();
}

size_t WebRtcVideoFrame::GetHeight() const {
  return video_frame_.Height();
}

const uint8* WebRtcVideoFrame::GetYPlane() const {
  WebRtc_UWord8* buffer = video_frame_.Buffer();
  return buffer;
}

const uint8* WebRtcVideoFrame::GetUPlane() const {
  WebRtc_UWord8* buffer = video_frame_.Buffer();
  if (buffer)
    buffer += (video_frame_.Width() * video_frame_.Height());
  return buffer;
}

const uint8* WebRtcVideoFrame::GetVPlane() const {
  WebRtc_UWord8* buffer = video_frame_.Buffer();
  if (buffer)
    buffer += (video_frame_.Width() * video_frame_.Height() * 5 / 4);
  return buffer;
}

uint8* WebRtcVideoFrame::GetYPlane() {
  WebRtc_UWord8* buffer = video_frame_.Buffer();
  return buffer;
}

uint8* WebRtcVideoFrame::GetUPlane() {
  WebRtc_UWord8* buffer = video_frame_.Buffer();
  if (buffer)
    buffer += (video_frame_.Width() * video_frame_.Height());
  return buffer;
}

uint8* WebRtcVideoFrame::GetVPlane() {
  WebRtc_UWord8* buffer = video_frame_.Buffer();
  if (buffer)
    buffer += (video_frame_.Width() * video_frame_.Height() * 5 / 4);
  return buffer;
}

VideoFrame* WebRtcVideoFrame::Copy() const {
  WebRtc_UWord8* buffer = video_frame_.Buffer();
  if (!buffer)
    return NULL;

  size_t new_buffer_size = video_frame_.Length();
  uint8* new_buffer = new uint8[new_buffer_size];
  memcpy(new_buffer, buffer, new_buffer_size);
  WebRtcVideoFrame* copy = new WebRtcVideoFrame();
  copy->Attach(new_buffer, new_buffer_size,
               video_frame_.Width(), video_frame_.Height(),
               pixel_width_, pixel_height_,
               elapsed_time_, video_frame_.TimeStamp(), rotation_);
  return copy;
}

bool WebRtcVideoFrame::MakeExclusive() {
  // WebRtcVideoFrame::Copy makes a deep copy of the frame buffer.  No action
  // is needed for MakeExclusive.
  return true;
}

size_t WebRtcVideoFrame::CopyToBuffer(uint8* buffer, size_t size) const {
  if (!video_frame_.Buffer()) {
    return 0;
  }

  size_t needed = video_frame_.Length();
  if (needed <= size) {
    memcpy(buffer, video_frame_.Buffer(), needed);
  }
  return needed;
}

size_t WebRtcVideoFrame::ConvertToRgbBuffer(uint32 to_fourcc,
                                            uint8* buffer,
                                            size_t size,
                                            size_t pitch_rgb) const {
  if (!video_frame_.Buffer()) {
    return 0;
  }

  size_t width = video_frame_.Width();
  size_t height = video_frame_.Height();
  // See http://www.virtualdub.org/blog/pivot/entry.php?id=190 for a good
  // explanation of pitch and why this is the amount of space we need.
  size_t needed = pitch_rgb * (height - 1) + 4 * width;

  if (needed > size) {
    LOG(LS_WARNING) << "RGB buffer is not large enough";
    return 0;
  }

  webrtc::VideoType to_type = webrtc::kUnknown;
  switch (to_fourcc) {
    case FOURCC_ARGB:
      to_type = webrtc::kARGB;
      break;
    default:
      LOG(LS_WARNING) << "RGB type not supported: " << to_fourcc;
      return 0;
  }

  if (to_type != webrtc::kUnknown) {
    webrtc::ConvertFromI420(to_type, video_frame_.Buffer(),
                            width, height, buffer);
  }

  return needed;
}

void WebRtcVideoFrame::StretchToPlanes(
    uint8* y, uint8* u, uint8* v,
    int32 dst_pitch_y, int32 dst_pitch_u, int32 dst_pitch_v,
    size_t width, size_t height, bool interpolate, bool crop) const {
  // TODO: Implement StretchToPlanes
}

size_t WebRtcVideoFrame::StretchToBuffer(size_t w, size_t h,
                                         uint8* buffer, size_t size,
                                         bool interpolate,
                                         bool crop) const {
  if (!video_frame_.Buffer()) {
    return 0;
  }

  size_t needed = video_frame_.Length();

  if (needed <= size) {
    uint8* bufy = buffer;
    uint8* bufu = bufy + w * h;
    uint8* bufv = bufu + ((w + 1) >> 1) * ((h + 1) >> 1);
    StretchToPlanes(bufy, bufu, bufv, w, (w + 1) >> 1, (w + 1) >> 1, w, h,
                    interpolate, crop);
  }
  return needed;
}

void WebRtcVideoFrame::StretchToFrame(VideoFrame* target,
    bool interpolate, bool crop) const {
  if (!target) return;

  StretchToPlanes(target->GetYPlane(),
                  target->GetUPlane(),
                  target->GetVPlane(),
                  target->GetYPitch(),
                  target->GetUPitch(),
                  target->GetVPitch(),
                  target->GetWidth(),
                  target->GetHeight(),
                  interpolate, crop);
  target->SetElapsedTime(GetElapsedTime());
  target->SetTimeStamp(GetTimeStamp());
}

VideoFrame* WebRtcVideoFrame::Stretch(size_t w, size_t h,
    bool interpolate, bool crop) const {
  // TODO: implement
  return NULL;
}

}  // namespace cricket
