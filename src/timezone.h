#ifndef TIMEZONE_H
#define TIMEZONE_H

#include <nan.h>
#include <string>
#include "time_zone.h"

class TimeZone : public Nan::ObjectWrap {
public:
	static void Init(v8::Local<v8::Object> target);
	static v8::Local<v8::Object> NewInstance();
	static v8::Local<v8::Object> NewInstance(v8::Local<v8::Value> arg);
	static Nan::Persistent<v8::FunctionTemplate> prototype;

	std::string name;
	cctz::time_zone value;

private:
	explicit TimeZone();
	~TimeZone();

	static Nan::Persistent<v8::Function> constructor;
	static NAN_METHOD(New);
	static NAN_METHOD(Lookup);

	static NAN_GETTER(GetName);
};

#endif
