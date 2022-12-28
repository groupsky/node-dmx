#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ftdi.h>
#include <sys/time.h>
#include <time.h>
#include "dmx.h"

int min(int a,int b) {
  return ( a < b ? a : b);
}
int cmp(int a,int b) {
  return ( a == b ? 0 : ( a > b ? 1 : -1 ));
}
void CalculateSleep (timespec *sleep, unsigned int Hz) {
  const long transmitTime = 21000000;
  long ns = 1000000000 / Hz - transmitTime;
  if (ns < 0) ns = 0;
  sleep->tv_sec = 0;
  sleep->tv_nsec = ns;
}

NAN_METHOD(list) {
  unsigned int i;
  int ret;
  struct ftdi_context ftdic;
  struct ftdi_device_list *devlist, *curdev;
  char manufacturer[128], description[128], serial[128];
  const size_t errln = 128;
  char err[errln];

  if (ftdi_init(&ftdic) < 0) {
    Nan::ThrowError(Nan::Error("ftdi_init failed"));
    return;
  }
  if ((ret = ftdi_usb_find_all(&ftdic, &devlist, 0x0403, 0x6001)) < 0) {
    snprintf(err, errln, "ftdi_usb_find_all failed: %d (%s)", ret, ftdi_get_error_string(&ftdic));
    Nan::ThrowError(Nan::Error(err));
    return;
  }
  v8::Local<v8::Array> result = Nan::New<v8::Array>(ret);
  v8::Local<v8::Object> obj;
  v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();

  i = 0;
  for (curdev = devlist; curdev != NULL; i++) {
      if ((ret = ftdi_usb_get_strings(&ftdic, curdev->dev, manufacturer, 128, description, 128, serial, 128)) < 0) {
        snprintf(err, errln, "ftdi_usb_get_strings failed: %d (%s)", ret, ftdi_get_error_string(&ftdic));
        Nan::ThrowError(Nan::Error(err));
        return;
      }
      obj = Nan::New<v8::Object>();
      obj->Set(context, Nan::New("manufacturer").ToLocalChecked(), Nan::New(manufacturer).ToLocalChecked());
      obj->Set(context, Nan::New("description").ToLocalChecked(), Nan::New(description).ToLocalChecked());
      obj->Set(context, Nan::New("serial").ToLocalChecked(), Nan::New(serial).ToLocalChecked());
      Nan::Set(result, i, obj);
      curdev = curdev->next;
  }

  ftdi_list_free(&devlist);
  ftdi_deinit(&ftdic);

  info.GetReturnValue().Set(result);
}

DMX::DMX() {};
DMX::~DMX() {};
Nan::Persistent<v8::Function> DMX::constructor;

NAN_MODULE_INIT(DMX::Init) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("DMX").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "start", Start);
  Nan::SetPrototypeMethod(tpl, "stop", Stop);
  Nan::SetPrototypeMethod(tpl, "set", Set);
  Nan::SetPrototypeMethod(tpl, "step", Step);
  Nan::SetPrototypeMethod(tpl, "setHz", SetHz);

  constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
  Nan::Set(target, Nan::New("DMX").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
}

NAN_METHOD(DMX::New) {
  if (!info.IsConstructCall()) {
    const int argc = 1;
    v8::Local<v8::Value> argv[argc] = {info[0]};
    v8::Local<v8::Function> cons = Nan::New(constructor);
    info.GetReturnValue().Set(Nan::NewInstance(cons, argc, argv).ToLocalChecked());
    return;
  }

  if (!info[0]->IsUndefined() && !info[0]->IsInt32()) {
    Nan::ThrowTypeError("devId must be an int");
    return;
  }

  // Initalize new object
  DMX* obj = new DMX();
  pthread_mutex_init(&(obj->lock), NULL);
  obj->portOpen = obj->threadRun = obj->changes = false;
  obj->step=255;
  CalculateSleep(&(obj->sleep),25);
  memset(obj->dmxVal,0,512);
  memset(obj->newVal,0,512);

  struct ftdi_context ftdic;
  struct ftdi_device_list *devlist, *curdev;
  const size_t errln = 128;
  char err[errln];
  int ret;
  unsigned int i = 0, devid = info[0]->IsUndefined() ? 0 : Nan::To<int>(info[0]).FromJust();

  // Find device
  if (ftdi_init(&ftdic) < 0) {
    Nan::ThrowTypeError("ftdi_init failed");
    return;
  }
  if ((ret = ftdi_usb_find_all(&ftdic, &devlist, 0x0403, 0x6001)) < 0) {
    snprintf(err, errln, "ftdi_usb_find_all failed: %d (%s)", ret, ftdi_get_error_string(&ftdic));
    ftdi_list_free(&devlist);
    ftdi_deinit(&ftdic);
    Nan::ThrowTypeError(err);
    return;
  }
  for (curdev = devlist; curdev != NULL; i++) {
    if (i == devid) break;
    curdev = curdev->next;
  }
  if (curdev == NULL) {
    snprintf(err, errln, "Cannot find device #%d", devid);
    ftdi_list_free(&devlist);
    ftdi_deinit(&ftdic);
    Nan::ThrowTypeError(err);
    return;
  }

  // Open device
  if ((ret = ftdi_usb_open_dev(&ftdic, curdev->dev)) < 0) {
    snprintf(err, errln, "ftdi_usb_open_dev failed: %d (%s)", ret, ftdi_get_error_string(&ftdic));
    ftdi_list_free(&devlist);
    ftdi_deinit(&ftdic);
    Nan::ThrowTypeError(err);
    return;
  }
  ftdi_list_free(&devlist);

  // Setup device
  if (((ret = ftdi_set_baudrate(&ftdic, 250000)) < 0) ||
      ((ret = ftdi_set_line_property2(&ftdic, BITS_8, STOP_BIT_2, NONE, BREAK_ON)) < 0) ||
      ((ret = ftdi_usb_purge_buffers(&ftdic)) < 0)) {
    snprintf(err, errln, "Can't setup device: %d (%s)", ret, ftdi_get_error_string(&ftdic));
    ftdi_deinit(&ftdic);
    Nan::ThrowTypeError(err);
    return;
  }

  obj->ftdic = ftdic;
  obj->portOpen = true;
  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

void *DMX::thread_func(void* arg) {
  DMX *obj = (DMX*) arg;
  int i;
  unsigned char StartCode = 0;
  obj->threadRun = true;

  while (obj->threadRun) {
    pthread_mutex_lock(&(obj->lock));
    // Check for the changes flag
    if (obj->changes) {
      obj->changes = false;
      for (i = 0; i < 512; i++) {
        // Change level for one step and put flag back it is not enough
        obj->dmxVal[i] += cmp(obj->newVal[i] ,obj->dmxVal[i]) * min(abs(obj->dmxVal[i] - obj->newVal[i]), obj->step);
        if (obj->dmxVal[i] != obj->newVal[i]) obj->changes = true;
      }
    }
    pthread_mutex_unlock(&(obj->lock));

    // Transmit DMX packet
    ftdi_set_line_property2(&obj->ftdic, BITS_8, STOP_BIT_2, NONE, BREAK_ON);
    ftdi_set_line_property2(&obj->ftdic, BITS_8, STOP_BIT_2, NONE, BREAK_OFF);
    ftdi_write_data(&obj->ftdic, &StartCode,  1);
    ftdi_write_data(&obj->ftdic, obj->dmxVal, 512);
    if (obj->sleep.tv_nsec > 0) nanosleep(&(obj->sleep), NULL);
  }

  return NULL;
}

NAN_METHOD(DMX::Start) {
  DMX *obj = Nan::ObjectWrap::Unwrap<DMX>(info.This());

  // Start working thread
  if (!obj->portOpen ||
      obj->threadRun ||
      (pthread_create(&(obj->thread), NULL, obj->thread_func, obj) != 0)
     ) {
    info.GetReturnValue().Set(false);
    return;
  }

  info.GetReturnValue().Set(true);
}

NAN_METHOD(DMX::Stop) {
  DMX *obj = Nan::ObjectWrap::Unwrap<DMX>(info.This());

  if (!obj->threadRun || !obj->portOpen) {
    info.GetReturnValue().Set(false);
    return;
  }

  obj->threadRun = false;

  // Wait while thread ends if we want
  if (Nan::To<bool>(info[0]).FromJust()) {
    pthread_join(obj->thread, NULL);
  }

  info.GetReturnValue().Set(true);
}

NAN_METHOD(DMX::Step) {
  DMX *obj = Nan::ObjectWrap::Unwrap<DMX>(info.This());

  int s = Nan::To<int>(info[0]).FromJust();
  if (s < 1 || s > 255) s=255;

  obj->step = s;

  info.GetReturnValue().Set(true);
}

NAN_METHOD(DMX::SetHz) {
  DMX *obj = Nan::ObjectWrap::Unwrap<DMX>(info.This());

  int Hz=Nan::To<int>(info[0]).FromJust();
  if (Hz < 1 || Hz > 50) {
    info.GetReturnValue().Set(false);
    return;
  }
  CalculateSleep(&(obj->sleep),Hz);

  info.GetReturnValue().Set(true);
}

NAN_METHOD(DMX::Set) {
  DMX *obj = Nan::ObjectWrap::Unwrap<DMX>(info.This());
  int i, l, val;
  bool setAll;
  v8::Local<v8::Array> arr;

  // Start the working thread if it is not
  if (!obj->threadRun) Start(info);

  // Set all channels to the same level if argument in not an Array
  if (info[0]->IsArray()) {
    setAll=false;
    arr = v8::Local<v8::Array>::Cast(info[0]);
    l = arr->Length();
    if (l > 512) l = 512;
  } else {
    setAll = true;
    val=Nan::To<int>(info[0]).FromJust();
    l = 512;
    if (val < 0) val = 0;
    if (val > 255) val = 255;
  }

  pthread_mutex_lock(&(obj->lock));
  for (i = 0; i < l; i++) {
    if (!setAll) {
      val = Nan::To<int>((Nan::Get(arr, i)).ToLocalChecked()).FromJust();
      if (val < 0) val = 0;
      if (val > 255) val = 255;
    }

    // Check for the changes
    if (obj->newVal[i] != val) {
      obj->newVal[i] = val;
      obj->changes = true;
    }
  }
  pthread_mutex_unlock(&(obj->lock));

  // sum is a number of non-zero channels
  int sum = 0;
  for (i = 0; i < 512; i++)
    sum += obj->newVal[i] > 0 ? 1 : 0;

  info.GetReturnValue().Set(sum);
}

NAN_MODULE_INIT(init) {
  DMX::Init(target);
  Nan::SetMethod(target, "list", list);
}

NODE_MODULE(dmx_native, init);
