//这是一种近乎固定的写法，当需要解析一些message，但是连任何消息都不知道时
//需要用到desc和ref配合，desc获取message对象的元信息，得到对应字段field的
//元信息，然后ref可以结合message基类指针与field直接获取真实的值


// 1. 拿描述符和反射器
auto desc = msg->GetDescriptor();
auto refl = msg->GetReflection();

// 2. 遍历字段
for (int i = 0; i < desc->field_count(); ++i) {
    auto field = desc->field(i);

    // 3. switch 判断类型
    switch (field->cpp_type()) {
        case FieldDescriptor::CPPTYPE_INT32:
            // 4. 用反射器读值
            val = refl->GetInt32(*msg, field);
            break;
        case FieldDescriptor::CPPTYPE_STRING:
            val = refl->GetString(*msg, field);
            break;
        // ... 其他类型
    }
}


// 伪代码：通用的“对象转SQL”函数
string GenerateInsertSQL(Message* msg) {
    string sql = "INSERT INTO table VALUES (";

    const Descriptor* desc = msg->GetDescriptor();
    const Reflection* refl = msg->GetReflection();

    // 自动遍历所有字段
    for (int i = 0; i < desc->field_count(); i++) {
        const FieldDescriptor* field = desc->field(i); // 拿到字段说明书

        // 利用反射，从 msg 对象里把这个字段的值“抠”出来
        if (field->type() == TYPE_INT32) {
            int val = refl->GetInt32(*msg, field);
            sql += to_string(val) + ", ";
        } else if (field->type() == TYPE_STRING) {
            string val = refl->GetString(*msg, field);
            sql += "'" + val + "', ";
        }
    }

    sql += ")";
    return sql;
}