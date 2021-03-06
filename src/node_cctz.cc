#include <string>
#include <chrono>
#include <nan.h>

#include "civil_time.h"
#include "time_zone.h"
#include "timezone.h"
#include "timepoint.h"
#include "civiltime.h"

NAN_METHOD(utc_time_zone) {
	v8::Local<v8::Object> result = TimeZone::NewInstance();
	TimeZone* tz = Nan::ObjectWrap::Unwrap<TimeZone>(result);
	tz->value = cctz::utc_time_zone();
	tz->name = "UTC";
	info.GetReturnValue().Set(result);
}

NAN_METHOD(local_time_zone) {
	v8::Local<v8::Object> result = TimeZone::NewInstance();
	TimeZone* tz = Nan::ObjectWrap::Unwrap<TimeZone>(result);
	tz->value = cctz::local_time_zone();
	tz->name = "localtime";
	info.GetReturnValue().Set(result);
}

NAN_METHOD(load_time_zone) {
	if (info.Length() < 1) {
		Nan::ThrowTypeError("Expected timezone name as first argument");
		return;
	}

	if (!info[0]->IsString()) {
		Nan::ThrowTypeError("timezone must be a string");
		return;
	}

	// TODO: Cache TimeZone objects (+100,000 op/s)
	info.GetReturnValue().Set(TimeZone::NewInstance(info[0]));
}

NAN_METHOD(parse) {
	if (info.Length() < 3) {
		Nan::ThrowTypeError("Expected 3 arguments: format, input, timezone");
		return;
	}

	if (!info[0]->IsString()) {
		Nan::ThrowTypeError("format must be a string");
		return;
	}

	if (!info[1]->IsString()) {
		Nan::ThrowTypeError("input must be a string");
		return;
	}

	auto arg2 = info[2]->ToObject();
	if (!Nan::New(TimeZone::prototype)->HasInstance(arg2)) {
		Nan::ThrowTypeError("timezone must be an instance of TimeZone");
		return;
	}

	std::string format = *Nan::Utf8String(info[0]);
	std::string input = *Nan::Utf8String(info[1]);

	TimeZone* tz = Nan::ObjectWrap::Unwrap<TimeZone>(arg2);
	v8::Local<v8::Object> result = TimePoint::NewInstance();
	TimePoint* tp = Nan::ObjectWrap::Unwrap<TimePoint>(result);

	bool parsed = cctz::parse(format, input, tz->value, &(tp->value));

	if (!parsed) {
		return;
	}

	info.GetReturnValue().Set(result);
}

NAN_METHOD(format) {
	if (info.Length() < 3) {
		Nan::ThrowTypeError("Expected 3 arguments: format, timepoint, timezone");
		return;
	}

	if (!info[0]->IsString()) {
		Nan::ThrowTypeError("format must be a string");
		return;
	}

	auto arg1 = info[1]->ToObject();
	if (!Nan::New(TimePoint::prototype)->HasInstance(arg1)) {
		Nan::ThrowTypeError("timepoint must be an instance of TimePoint");
		return;
	}

	auto arg2 = info[2]->ToObject();
	if (!Nan::New(TimeZone::prototype)->HasInstance(arg2)) {
		Nan::ThrowTypeError("timezone must be an instance of TimeZone");
		return;
	}

	std::string format = *Nan::Utf8String(info[0]);
	TimePoint* tp = Nan::ObjectWrap::Unwrap<TimePoint>(arg1);
	TimeZone* tz = Nan::ObjectWrap::Unwrap<TimeZone>(arg2);
	std::string str = cctz::format(format, tp->value, tz->value);

	info.GetReturnValue().Set(Nan::New(str).ToLocalChecked());
}

NAN_METHOD(convert) {
	if (info.Length() < 2) {
		Nan::ThrowTypeError("Expected 2 arguments: timepoint/civilseconds, timezone");
		return;
	}

	auto arg1 = info[1]->ToObject();
	if (!Nan::New(TimeZone::prototype)->HasInstance(arg1)) {
		Nan::ThrowTypeError("timezone must be an instance of TimeZone");
		return;
	}

	TimeZone* tz = Nan::ObjectWrap::Unwrap<TimeZone>(arg1);

	auto arg0 = info[0]->ToObject();
	if (Nan::New(TimePoint::prototype)->HasInstance(arg0)) {
		TimePoint* tp = Nan::ObjectWrap::Unwrap<TimePoint>(arg0);
		auto al = tz->value.lookup(tp->value);

		v8::Local<v8::Object> csObj = CivilSecond::NewInstance();
		CivilSecond* cs = Nan::ObjectWrap::Unwrap<CivilSecond>(csObj);
		cs->value = al.cs;

		info.GetReturnValue().Set(csObj);
		return;
	}

	if (Nan::New(CivilSecond::prototype)->HasInstance(arg0)) {
		CivilSecond* cs = Nan::ObjectWrap::Unwrap<CivilSecond>(arg0);
		const cctz::time_zone::civil_lookup cl = tz->value.lookup(cs->value);

		v8::Local<v8::Object> tpObj = TimePoint::NewInstance();
		TimePoint* tp = Nan::ObjectWrap::Unwrap<TimePoint>(tpObj);

		if (cl.kind == cctz::time_zone::civil_lookup::SKIPPED) {
			tp->value = cl.trans;
		} else {
			tp->value = cl.pre;
		}

		info.GetReturnValue().Set(tpObj);
		return;
	}

	Nan::ThrowTypeError("first argument should be instance of TimePoint or CivilSecond");
}

NAN_MODULE_INIT(Init) {
	TimePoint::Init(target);
	TimeZone::Init(target);
	CivilSecond::Init(target);

	NAN_EXPORT(target, load_time_zone);
	NAN_EXPORT(target, utc_time_zone);
	NAN_EXPORT(target, local_time_zone);
	NAN_EXPORT(target, parse);
	NAN_EXPORT(target, format);
	NAN_EXPORT(target, convert);
}

NODE_MODULE(node_cctz, Init)
