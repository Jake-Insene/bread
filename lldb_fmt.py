import lldb

class SliceSyntheticProvider:
    def __init__(self, valobj, _):
        self.valobj = valobj
        self.update()

    def update(self):
        self.items = self.valobj.GetChildMemberWithName("items")
        self.len = self.valobj.GetChildMemberWithName("len").GetValueAsUnsigned()

    def num_children(self):
        return self.len

    def get_child_at_index(self, index):
        if index >= self.len:
            return None

        element_type = self.items.GetType().GetPointeeType()
        base_addr = self.items.GetValueAsUnsigned()
        element_size = element_type.GetByteSize()
        addr = base_addr + index * element_size

        return self.items.CreateValueFromAddress(
            f"[{index}]",
            addr,
            element_type
        )

    def has_children(self):
        return True

class ArraySyntheticProvider:
    def __init__(self, valobj, _):
        self.valobj = valobj
        self.update()

    def update(self):
        self.items = self.valobj.GetChildMemberWithName("items").GetChildMemberWithName("items")
        self.count = self.valobj.GetChildMemberWithName("count").GetValueAsUnsigned()

    def num_children(self):
        return self.count

    def get_child_at_index(self, index):
        if index >= self.count:
            return None

        element_type = self.items.GetType().GetPointeeType()
        base_addr = self.items.GetValueAsUnsigned()
        element_size = element_type.GetByteSize()
        addr = base_addr + index * element_size

        return self.items.CreateValueFromAddress(
            f"[{index}]",
            addr,
            element_type
        )

    def has_children(self):
        return True

class StaticArraySyntheticProvider:
    def __init__(self, valobj, _):
        self.valobj = valobj
        self.update()

    def update(self):
        self.items = self.valobj.GetChildMemberWithName("items")
        self.count = self.valobj.GetChildMemberWithName("count").GetValueAsUnsigned()

    def num_children(self):
        return self.count

    def get_child_at_index(self, index):
        if index >= self.count:
            return None

        return self.items.GetChildAtIndex(index)

    def has_children(self):
        return True

class StackSyntheticProvider:
    def __init__(self, valobj, _):
        self.valobj = valobj
        self.update()

    def update(self):
        self.items = self.valobj.GetChildMemberWithName("items").GetChildMemberWithName("items")
        self.sp = self.valobj.GetChildMemberWithName("sp").GetValueAsUnsigned()

    def num_children(self):
        return self.sp

    def get_child_at_index(self, index):
        if index >= self.sp:
            return None

        element_type = self.items.GetType().GetPointeeType()
        base_addr = self.items.GetValueAsUnsigned()
        element_size = element_type.GetByteSize()
        addr = base_addr + index * element_size

        return self.items.CreateValueFromAddress(
            f"[{index}]",
            addr,
            element_type
        )

    def has_children(self):
        return True


def slice_summary(valobj, _):
    length = valobj.GetNonSyntheticValue().GetChildMemberWithName("len").GetValue()
    return f"{{ len={length} }}"

def array_summary(valobj, _):
    count = valobj.GetNonSyntheticValue().GetChildMemberWithName("count").GetValue()
    return f"{{ count={count} }}"

def static_array_summary(valobj, _):
    count = valobj.GetNonSyntheticValue().GetChildMemberWithName("count").GetValue()
    return f"{{ count={count} }}"

def stack_summary(valobj, _):
    sp = valobj.GetNonSyntheticValue().GetChildMemberWithName("sp").GetValue()
    return f"{{ sp={sp} }}"

def string_view_summary(valobj, _):
    obj = valobj.GetNonSyntheticValue()
    items = obj.GetChildMemberWithName("items").GetValueAsUnsigned()
    length = obj.GetChildMemberWithName("len").GetValueAsUnsigned()
    if items == 0 or length == 0:
        return '""'
    
    error = lldb.SBError()
    process = lldb.debugger.GetSelectedTarget().GetProcess()
    content = process.ReadMemory(items, length, error)
    if error.Success():
        return f'"{content.decode("utf-8")}"'
    return '""'

def vector2_summary(valobj, _):
    x = valobj.GetNonSyntheticValue().GetChildMemberWithName("x").GetValue()
    y = valobj.GetNonSyntheticValue().GetChildMemberWithName("y").GetValue()
    return f"{{ x = {x}, y = {y} }}"

def vector3_summary(valobj, _):
    x = valobj.GetNonSyntheticValue().GetChildMemberWithName("x").GetValue()
    y = valobj.GetNonSyntheticValue().GetChildMemberWithName("y").GetValue()
    z = valobj.GetNonSyntheticValue().GetChildMemberWithName("z").GetValue()
    return f"{{ x = {x}, y = {y}, z = {z} }}"

def vector4_summary(valobj, _):
    x = valobj.GetNonSyntheticValue().GetChildMemberWithName("x").GetValue()
    y = valobj.GetNonSyntheticValue().GetChildMemberWithName("y").GetValue()
    z = valobj.GetNonSyntheticValue().GetChildMemberWithName("z").GetValue()
    w = valobj.GetNonSyntheticValue().GetChildMemberWithName("w").GetValue()
    return f"{{ x = {x}, y = {y}, z = {z}, w = {w} }}"

def __lldb_init_module(debugger, _):
    # Slice<T>
    debugger.HandleCommand(
        "type synthetic add -x '^Slice<.*>$' "
        "-l lldb_fmt.SliceSyntheticProvider"
    )

    debugger.HandleCommand(
        "type summary add -x '^Slice<.*>$' "
        "-F lldb_fmt.slice_summary"
    )

    # Array<T>
    debugger.HandleCommand(
        "type synthetic add -x '^Collections::Array<.*>$' "
        "-l lldb_fmt.ArraySyntheticProvider"
    )

    debugger.HandleCommand(
        "type summary add -x '^Collections::Array<.*>$' "
        "-F lldb_fmt.array_summary"
    )

    # Stack<T>
    debugger.HandleCommand(
        "type synthetic add -x '^Collections::Stack<.*>$' "
        "-l lldb_fmt.StackSyntheticProvider"
    )

    debugger.HandleCommand(
        "type summary add -x '^Collections::Stack<.*>$' "
        "-F lldb_fmt.stack_summary"
    )

    # StringView
    debugger.HandleCommand(
        "type summary add -x '^Collections::BaseStringView<.*>$' "
        "-F lldb_fmt.string_view_summary"
    )

    # Math
    debugger.HandleCommand(
        "type summary add -x '^Vector2T<.*>$' "
        "-F lldb_fmt.vector2_summary"
    )

    debugger.HandleCommand(
        "type summary add -x '^Vector3T<.*>$' "
        "-F lldb_fmt.vector3_summary"
    )

    debugger.HandleCommand(
        "type summary add -x '^Vector4T<.*>$' "
        "-F lldb_fmt.vector4_summary"
    )
