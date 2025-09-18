#pragma once

#define VTFunc(ret, name, ...) ret(*name)(__VA_ARGS__)

#define VTFuncDef(name) void name() { vtable.name(); }
#define VTFuncDefS(name) static void name() { vtable.name(); }

#define VTFuncDefRet(ret, name) [[nodiscard]] ret name() { return vtable.name(); }
#define VTFuncDefRetS(ret, name) [[nodiscard]] static ret name() { return vtable.name(); }

#define VTFuncDefArg1(name, arg1) void name(arg1 _1) { vtable.name(_1); }
#define VTFuncDefArg1S(name, arg1) static void name(arg1 _1) { vtable.name(_1); }

#define VTFuncDefArg2(name, arg1, arg2) void name(arg1 _1, arg2 _2) { vtable.name(_1, _2); }
#define VTFuncDefArg2S(name, arg1, arg2) static void name(arg1 _1, arg2 _2) { vtable.name(_1, _2); }

#define VTFuncDefArg3(name, arg1, arg2, arg3) void name(arg1 _1, arg2 _2, arg3 _3) { vtable.name(_1, _2, _3); }
#define VTFuncDefArg3S(name, arg1, arg2, arg3) static void name(arg1 _1, arg2 _2, arg3 _3) { vtable.name(_1, _2, _3); }

#define VTFuncDefArg1Ret(ret, name, arg1) [[nodiscard]] ret name(arg1 _1) { return vtable.name(_1); }
#define VTFuncDefArg1RetS(ret, name, arg1) [[nodiscard]] static ret name(arg1 _1) { return vtable.name(_1); }

#define VTFuncDefArg2Ret(ret, name, arg1, arg2) [[nodiscard]] ret name(arg1 _1, arg2 _2) { return vtable.name(_1, _2); }
#define VTFuncDefArg2RetS(ret, name, arg1, arg2) [[nodiscard]] static ret name(arg1 _1, arg2 _2) { return vtable.name(_1, _2); }

#define VTableCall(self, fn, ...) \
    (*self.*self->vtable.fn)(__VA_ARGS__)

#define VTCastGet(field, func) (decltype(field))&func

