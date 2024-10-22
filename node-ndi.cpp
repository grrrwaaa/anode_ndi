#include <cstdio>
#include <chrono>
#include <cstddef>
#include <Processing.NDI.Lib.h>

#ifdef _WIN32
#ifdef _WIN64
#pragma comment(lib, "Processing.NDI.Lib.x64.lib")
#else // _WIN64
#pragma comment(lib, "Processing.NDI.Lib.x86.lib")
#endif // _WIN64
#endif // _WIN32

#include "napi.h"

struct Sender : public Napi::ObjectWrap<Sender> {

    NDIlib_send_create_t settings;
	NDIlib_send_instance_t sender;
	NDIlib_video_frame_v2_t frame;

    Sender(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Sender>(info) {
		Napi::Env env = info.Env();
		Napi::Object This = info.This().As<Napi::Object>();
        if (info.Length() > 0 && info[0].IsString()) {
            settings.p_ndi_name = info[0].ToString().Utf8Value().c_str();
        } // TODO else throw error
        sender = NDIlib_send_create(&settings);

        // initialize frame:
        frame.xres = 320;
		frame.yres = 240;
		frame.FourCC = NDIlib_FourCC_video_type_BGRA;  // 4:4:4:4 
		//frame.FourCC = NDIlib_FourCC_video_type_RGBA;  // 4:4:4:4 
		frame.frame_rate_N = 30;
		frame.frame_rate_D = 1;
		frame.picture_aspect_ratio = float(frame.xres)/float(frame.yres);
		frame.frame_format_type = NDIlib_frame_format_type_progressive;
		// The timecode of this frame in 100-nanosecond intervals.
		frame.timecode = 0;  // int64_t
		// // The video data itself.
		frame.p_data = nullptr;
		// union {	// If the FourCC is not a compressed type, then this will be the inter-line stride of the video data
		// 	// in bytes.  If the stride is 0, then it will default to sizeof(one pixel)*xres.
		// 	int line_stride_in_bytes;
		// 	// If the FourCC is a compressed type, then this will be the size of the p_data buffer in bytes.
		// 	int data_size_in_bytes;
		// };
		frame.line_stride_in_bytes = 0;
		// // Per frame metadata for this frame. This is a NULL terminated UTF8 string that should be in XML format.
		// // If you do not want any metadata then you may specify NULL here.
		// const char* p_metadata; // Present in >= v2.5
		frame.p_metadata = NULL;
	}

    ~Sender() {
		printf("release NDI sender\n");
        NDIlib_send_destroy(sender);
        //sender.ReleaseSender();
    }

    // setData arguments:
    // data (uint8array)
    // width, height
    Napi::Value setData(const Napi::CallbackInfo& info) {
		Napi::Env env = info.Env();
		Napi::Object This = info.This().As<Napi::Object>();

        // update resolution:
        if (info[1].IsNumber()) frame.xres = info[1].ToNumber().Uint32Value();
        if (info[2].IsNumber()) frame.yres = info[2].ToNumber().Uint32Value();

        Napi::TypedArrayOf<uint8_t> arr;
        if (info[0].IsTypedArray()) arr = info[0].As<Napi::TypedArrayOf<uint8_t>>();

        // verify it is large enough:
        if (arr.ByteLength() < frame.xres * frame.yres * 4) Napi::Error::New(env, "too few bytes for resolution").ThrowAsJavaScriptException();
        frame.p_data = arr.Data();
        
        return This;
    }

    // optionally with data:
    Napi::Value send(const Napi::CallbackInfo& info) {
		Napi::Env env = info.Env();
		Napi::Object This = info.This().As<Napi::Object>();

        if (info.Length() > 0) {
            setData(info);
        }

        NDIlib_send_send_video_v2(sender, &frame);
        
        return This;
    }

};

// Napi::Value find(const Napi::CallbackInfo& info) {
//     Napi::Env env = info.Env();

//     NDIlib_find_create_t find_create;
//     find_create.show_local_sources = true;
//     find_create.p_groups = nullptr;
//     find_create.p_extra_ips = nullptr;

//     auto handle = NDIlib_find_create2(&find_create);
//     if (!handle) {
//         Napi::Error::New(info.Env(), "Failed to initialize NDI finder").ThrowAsJavaScriptException();
//         return env.Null();
//     }

//     uint32_t count = 0;
//     const NDIlib_source_t *sources = NDIlib_find_get_current_sources(handle, &count);

//     if (!sources || count == 0) return Napi::Array::New(env, 0);

//     Napi::Array result = Napi::Array::New(env, count);
//     for (size_t i = 0; i < count; i++)
//     {
//         const NDIlib_source_t * source = &sources[i];

//         Napi::Object object = Napi::Object::New(env);

//         object.Set("name", source->p_ndi_name);
//         object.Set("urlAddress", source->p_url_address);
//         object.Set("ipAddress", source->p_ip_address);
//         object.Set("source", Napi::External<void>::New(env, (void *)source));

//         result[i] = object;
//     }

//     if (handle) NDIlib_find_destroy(handle);

//     return result;
// }



struct Receiver : public Napi::ObjectWrap<Receiver> {

    NDIlib_recv_create_v3_t receiveConfig;
    NDIlib_recv_instance_t recv;
	NDIlib_video_frame_v2_t frame;
    NDIlib_source_t* source = nullptr;
    uint32_t bytelength;

    Receiver(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Receiver>(info) {
		Napi::Env env = info.Env();
		Napi::Object This = info.This().As<Napi::Object>();

        auto options = info[0].As<Napi::Object>();
        This.Set("name", options.Get("name"));
        This.Set("urlAddress", options.Get("urlAddress"));
        This.Set("ipAddress", options.Get("ipAddress"));

        auto frame = This.Set("frame", Napi::Object::New(env));

        source = (NDIlib_source_t *)options.Get("source").As<Napi::External<void>>().Data();

        receiveConfig.p_ndi_recv_name = options.Get("name").ToString().Utf8Value().c_str();
        receiveConfig.color_format = NDIlib_recv_color_format_BGRX_BGRA;
        receiveConfig.bandwidth = NDIlib_recv_bandwidth_highest;
        receiveConfig.allow_video_fields = true;

        
        recv = NDIlib_recv_create_v3(&receiveConfig);

        NDIlib_recv_connect(recv, source);
	}

    ~Receiver() {
		printf("release NDI Receiver\n");
        //NDIlib_send_destroy(sender);
        //sender.ReleaseSender();
    }

    // arg: timeout
    Napi::Value video(const Napi::CallbackInfo& info) {
		Napi::Env env = info.Env();
		Napi::Object This = info.This().As<Napi::Object>();

        uint32_t timeout_ms = info[0].ToNumber().Uint32Value();

        auto Frame = This.Get("frame").ToObject();

        // This function will return NDIlib_frame_type_none if no data is received within the specified timeout and
        // NDIlib_frame_type_error if the connection is lost. Buffers captured with this must be freed with the
        // appropriate free function below.
        auto res = NDIlib_recv_capture_v2(recv, &frame, nullptr, nullptr, timeout_ms);
        switch (res) {
            // no data received
            // case NDIlib_frame_type_none: return env.Null();
            case NDIlib_frame_type_error: printf("lost connection\n"); return env.Null();
            case NDIlib_frame_type_video: break;
            default: return env.Null();
        }

        Frame.Set("xres", frame.xres);
        Frame.Set("yres", frame.yres);
        bytelength = frame.xres * frame.yres * 4;
        Frame.Set("bytelength", bytelength);

        // reallocate our buffer?
        Napi::TypedArrayOf<uint8_t> arr;
        auto arr_value = Frame.Get("data");
        if (arr_value.IsTypedArray()) {
            arr = arr_value.As<Napi::TypedArrayOf<uint8_t>>();
            // if the bytelength is wrong:
            if (arr.ByteLength() != bytelength) {
                arr = Napi::TypedArrayOf<uint8_t>::New(env, bytelength);
                Frame.Set("data", arr);
            }
        } else {
            // create one:
            arr = Napi::TypedArrayOf<uint8_t>::New(env, bytelength);
            Frame.Set("data", arr);
        }

        // now we can copy frame.p_data into arr:
        memcpy(arr.Data(), frame.p_data, bytelength);

        return Frame;
    }
};


struct Finder : public Napi::ObjectWrap<Finder> {

    NDIlib_find_create_t find_create;
    NDIlib_find_instance_t handle;
  
    Finder(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Finder>(info) {
		Napi::Env env = info.Env();
		Napi::Object This = info.This().As<Napi::Object>();

        find_create.show_local_sources = true;
        find_create.p_groups = "";
        find_create.p_extra_ips = "";

        handle = NDIlib_find_create2(&find_create);
        if (!handle) {
            Napi::Error::New(info.Env(), "Failed to initialize NDI finder").ThrowAsJavaScriptException();
        }
	}

    ~Finder() {
		printf("release NDI Finder\n");
        if (handle) NDIlib_find_destroy(handle);
    }

    Napi::Value find(const Napi::CallbackInfo& info) {
		Napi::Env env = info.Env();
		Napi::Object This = info.This().As<Napi::Object>();

        uint32_t count = 0;
        const NDIlib_source_t *sources = NDIlib_find_get_current_sources(handle, &count);

        if (!sources || count == 0) return Napi::Array::New(env, 0);

        printf("Found %d sources\n", count);

        Napi::Array result = Napi::Array::New(env, count);
        for (size_t i = 0; i < count; i++)
        {
            const NDIlib_source_t * source = &sources[i];

            Napi::Object object = Napi::Object::New(env);

            object.Set("name", source->p_ndi_name);
            object.Set("urlAddress", source->p_url_address);
            object.Set("ipAddress", source->p_ip_address);
            object.Set("source", Napi::External<void>::New(env, (void *)source));

            result[i] = object;
        }

        return result;
    }

};

class Module : public Napi::Addon<Module> {
public:
	
	Module(Napi::Env env, Napi::Object exports) {

		// See https://github.com/nodejs/node-addon-api/blob/main/doc/class_property_descriptor.md
		DefineAddon(exports, {
        //    Napi::PropertyDescriptor::Function("find", find),

		// 	// InstanceMethod("start", &Module::start),
		// 	// InstanceMethod("end", &Module::end),
		// 	// //InstanceMethod("test", &Module::test),
		// 	// // InstanceValue
		// 	// // InstanceAccessor
		// 	InstanceAccessor<&Module::devices>("devices"),
		// 	// InstanceAccessor<&Module::Gett>("t"),
		// 	// InstanceAccessor<&Module::GetSamplerate>("samplerate"),
		});
		
		{
            // This method is used to hook the accessor and method callbacks
            Napi::Function ctor = Sender::DefineClass(env, "Sender", {
                Sender::InstanceMethod<&Sender::setData>("setData"),
                Sender::InstanceMethod<&Sender::send>("send"),
            });

            // Create a persistent reference to the class constructor.
            Napi::FunctionReference* constructor = new Napi::FunctionReference();
            *constructor = Napi::Persistent(ctor);
            exports.Set("Sender", ctor);
            env.SetInstanceData<Napi::FunctionReference>(constructor);
        }
		
		{
            // This method is used to hook the accessor and method callbacks
            Napi::Function ctor = Finder::DefineClass(env, "Finder", {
                Finder::InstanceMethod<&Finder::find>("find"),
            });

            // Create a persistent reference to the class constructor.
            Napi::FunctionReference* constructor = new Napi::FunctionReference();
            *constructor = Napi::Persistent(ctor);
            exports.Set("Finder", ctor);
            env.SetInstanceData<Napi::FunctionReference>(constructor);
        }

        {
            // This method is used to hook the accessor and method callbacks
            Napi::Function ctor = Receiver::DefineClass(env, "Receiver", {
                Receiver::InstanceMethod<&Receiver::video>("video"),
                // Receiver::InstanceMethod<&Receiver::send>("send"),
            });

            // Create a persistent reference to the class constructor.
            Napi::FunctionReference* constructor = new Napi::FunctionReference();
            *constructor = Napi::Persistent(ctor);
            exports.Set("Receiver", ctor);
            env.SetInstanceData<Napi::FunctionReference>(constructor);
        }

        //exports.Set(Napi::String::New(env, "find"), Napi::Function::New(env, find));

	}
};

NODE_API_ADDON(Module)