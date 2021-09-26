#ifndef DMX_H
#define DMX_H

#include <nan.h>
#include <ftdi.h>


class DMX : public Nan::ObjectWrap {
 public:
  static NAN_MODULE_INIT(Init);

 private:
  DMX();
  ~DMX();

  static void *thread_func(void* arg);
  struct ftdi_context ftdic;
  pthread_t thread;
  pthread_mutex_t lock;
  unsigned char step;
  bool portOpen;
  bool threadRun;
  bool changes;
  unsigned char dmxVal[512];
  unsigned char newVal[512];
  struct timespec sleep;
  static NAN_METHOD(New);
  static NAN_METHOD(Start);
  static NAN_METHOD(Stop);
  static NAN_METHOD(Set);
  static NAN_METHOD(Step);
  static NAN_METHOD(SetHz);
  static Nan::Persistent<v8::Function> constructor;
};

#endif
