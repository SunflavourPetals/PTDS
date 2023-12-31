# PTDS 概念和规范 

## 注释

使用`["` `"]`进行注释, 其中`["`, `"]`中不应有任何字符将其分割,  
例如`[" 这是一个注释 "]` 是注释;  
而`[ "这不是一个注释 而是一个错误" ]` 不是注释, 因为`[`与`"`间、`"`与`]`间存在空格.  
`["` `"]`是多行注释.  
```
[" + - - - - - -+
   |  这是注释  |
   | 这也是注释 |
   + - - - - - -+ "]
```
```
[" 这还是一条注释 "]
```

### 拓展

拓展多行注释`("` `")`, `/*` `*/`, 和`["` `"]`效果相同.  
拓展单行注释`//`, 和 C++ 中的单行注释一样.  
```
[sth] 1, 2, 3 // 这是注释  
[prompt] "这不是注释, 上一行\"//\"后的注释到行尾已经截止了"  
```

## 值的表示

### 整数

整数可以使用十进制、十六进制(0x/0X)和二进制(0b/0B), 不支持使用八进制,  
正确示范如`123, +1, - 1, -0x0000fFaA, +0XFF00, +0b10000001, -0B0001`  
注意, 在使用十进制时, 不能以数字`0`开头,  
错误示范如`0123, -01, 00`等.  

### 浮点数

浮点数只能用十进制的方式表示, 小数点可以作为开头或结尾, 也可以没有小数点(解析器需要提前知道它的类型, 否则会将不带小数点的整数解析为整型), 但是不允许小数点前后都没有数字,  
正确示范如`1.0, + 1.0, -1.0, +.5, -1., .25, 16., 1, -1`  
错误示范如`0x7f, 0b0001, 0x1.1, ., -., +.`等

### 字符和字符串

字符和字符串需要使用引号`"`引起来, 值为字符类型时, 引号内有且只能有一个字符, 为了区别字符与字符串, 此处指明了值的类型并指定了实体名称,  
正确示范如  
`[char] <char> {    "C"   }`  
`[str]  <str>  {    "S"   }`  
`[str]  <str>  {    ""    }`  
`[str]  <str>  { "String" }`   
错误示范如  
`[char] <char> {    ""    }`  
`[char] <char> { "String" }`  

在字符串中换行的问题:  
1. 直接换行将导致换行符被录入
```
[str] "abc
def"
// Entity(L"str") -> "abc\r\ndef"(Windows使用CRLF)
```
2. 不想将文本文件中的换行录入的话可在行尾使用反斜杠`\`
```
[str2] "abc\
def"
// Entity(L"str2") -> "abcdef"
```
3. 想要在字符值或字符串中使用一些特殊符号如换行, 请使用[转义字符](#转义字符)
```
[str3] "abc\ndef"
// Entity(L"str3") -> "abc\ndef" 此处的换行符为\n, 转义的效果参照 C/C++: L"abc\ndef"  
```

#### 转义字符

PTDS 支持 C 语言中绝大部分的转义字符  
| Escape Character | HEX | DEC | DESC | 含义 |  
| :---: | :---: | :---: | :---: | :---: |  
| `\0` | `\x0000` | 0 | NULL | 空字符 |  
| `\a` | `\x0007` | 7 | BEL | 响铃 |  
| `\b` | `\x0008` | 8 | BS | 退格 |  
| `\f` | `\x000c` | 12 | FF | 换页符 |  
| `\n` | `\x000a` | 10 | LF | 换行符 |  
| `\r` | `\x000d` | 13 | CR | 回车符 |  
| `\t` | `\x0009` | 9 | HT | 水平制表符 |  
| `\v` | `\x000b` | 11 | VT | 垂直制表符 |  
| `\'` | `\x0027` | 39 | `'` | 单引号 |  
| `\"` | `\x0022` | 34 | `"` | 双引号 |  
| `\?` | `\x003f` | 63 | `?` | 问号 |  
| `\\` | `\x005c` | 92 | `\` | 反斜杠 |  
| `\xhhhh` | `\xhhhh` | -- | any | 任意字符 |  

不支持 `\ddd`(ddd为八进制数)用来指定字符,  
仅支持 `\xhhhh`(hhhh为1~4位十六进制数)

如 `"ABC\x20XYZ"` `"ABC\x0020XYZ"` 都表示`"ABC XYZ"`
但要注意, `\x`后最多可以有四位十六进制数, 如果想要表示`"ABC ABC"`, 却使用了`"ABC\x20ABC"`, 将达不到预期的效果(`\x20AB`被当成了一个字符), 使用`"ABC\x0020ABC"才能达到预期效果`.  

转义失败时, 忽略`\`  
如`"ABC\xXYZ"`中`\x`后没有十六进制数, 转义失败, 忽略`\`, 即解释为`"ABCxXYZ"`.  

### 布尔值

布尔值有专门的表示方法: `true` `t` `false` `f` (不检查大小写)  
也可以使用整数表示布尔值, 其中非零值表示`true`, `0`表示`false`
布尔值类型的示范  
`[bool] { true, false, t, f, TrUe, FaLsE, T, F, 1, 0, + 0x1, - 0b0000 }`
`[bool]<bool>{ 1, 0, true, F }`

## 实体

实体是一或多个具有相同[基本类型](#基本类型)的值组成的数组, 可以将实体写在一对大括号中, 也可以直接写出  
如`1, 2, 3`, `1`, `{ 1, 2, 3 }`, `{ 1 }`, `{ "a", "b", "c" }`

在 PTDS 中实体必须有名字, 作为实体的标识符, 实体的名字由[标签](#标签)指定.  
如`[array] { 1, 2, 3 }`  

在 c++ 程序中, 使用`Petal::PTDS::Entity(name)`等接口查询实体

### 实体名称

实体名称由自定义的标识符和`:`组成  
自定义的标识符不可以使用以下字符  
1. 控制字符, 即`\x0000`到`\x001f`和`\x007f`, 包括制表符, 换行符, `L'\0'`等
2. 空格 ` `
3. 及以下字符 `[` `]` `<` `>` `{` `}` `(` `)` `"` `,` `\` `/` `:`  

其余字符都可以使用,  
合法的实体名称片段如`something` `未命名` `あいしてる` `str1` `1` `.-_-_-_-_-...-` 等  

使用[嵌套标签](#标签)声明的名称按标签声明的顺序, 用`:`连接各个名称  
如  
```
[" PTDS "]
[First-name] [Last-name] 1, 2, 3
```
在 C++ 程序中查询则使用全名 `First-name:Last-name`  
``` c++
void query(const Petal::PTDS& ptds) try {
    ptds.Entity(L"First-name:Last-name");
}
catch (Petal::PTDSQueryException&) {
    std::cout << "can not fine entity: ";
    std::wcout << L"First-name:Last-name" << std::endl;
}
```
供查询实体时使用.  

## 标签

标签由方括号`[]`声明, 标签定义实体的一部分名称.  
如`[name] { 1, 2, 3 }`  
关于命名规则, 可参考[实体名称](#实体名称).  
标签可以嵌套声明,  
如  
```
[" 不带大括号的嵌套 "]
[First-name][Last-name] { 4, 5, 6 }
[" 带大括号时, 大括号内的所有实体都将获得这个标签内的名字 "]
[Window] {
    [Title] "Something..."
    [Date] {
        [Y] 2023
        [M] 6
        [D] 15
    }
}
```
嵌套声明标签时值和标签声明不能共存,  
错误案例如  
```
[something] {
    [array] { 1, 2, 3 }
    "something" // 错误, 不能存在属性
}
[something2] {
    "something"
    [array] { 1, 2, 3 } // 错误, 解析器会认为是大括号未闭合
}
```
同一个块(大括号围住的范围)内不允许出现相同的名称,  
错误案例如  
```
[array] { 1, 2, 3 }  // 定义了名称 array
[array] { 4, 5 }     // 错误
[sth] {              // 定义了名称 sth
    [1] { 1 }        // 定义了名称 sth:1
    [1] { 1 }        // 错误
}
[sth] { "a" }        // 错误
```
相对应的, 在 c++ 程序中查询由嵌套的标签命名的实体时, 需要提供实体的全名, 对上面的例子, 
应  
``` c++
void query_entity(const Petal::PTDS& ptds)
{
    ptds.Entity(L"First-name:Last-name");
    ptds.Entity(L"Window:Title");
    ptds.Entity(L"Window:Date:Y");
    ptds.Entity(L"Window:Date:M");
    ptds.Entity(L"Window:Date:D");
}
```

## 类型

### 基本类型

PTDS 基本类型表  
| PTDS Basic Type | PTDSBasicType/Enum | Enum Value | Type | Type in C++ |  
| :---: | :---: | :---: | :---: | :---: |  
| `u8` | `u8` | 1 | `uint8` | `unsigned char` |  
| `u16` | `u16` | 2 | `uint16` | `unsigned short` |  
| `u32` | `u32` | 3 | `uint32` | `unsigned long` |  
| `u64` | `u64` | 4 | `uint64` | `unsigned long long` |  
| `i8` | `i8` | 5 | `int8` | `signed char` |  
| `i16` | `i16` | 6 | `int16` | `short` |  
| `i32` | `i32` | 7 | `int32` | `long` |  
| `i64` | `i64` | 8 | `int64` | `long long` |  
| `f32` | `f32` | 9 | `float32` | `float` |  
| `f64` | `f64` | 10 | `float64` | `double` |  
| `char` | `cha` | 11 | `char16` | `wchar_t` |  
| `str` | `str` | 12 | `string` | `std::wstring` |  
| `bool` | `bln` | 16 | `boolean` | `bool` |  

PTDS 的基本类型有13个, 分别是  
1. `u8`, 对应 c++ `unsigned char` 类型.  
  数值范围在`0`~`255`  
2. `u16`, 对应 c++ `unsigned short int` 类型.  
  数值范围在`0`~`65,535`  
3. `u32`, 对应 c++ `unsigned long int` 类型.  
  数值范围在`0`~`4,294,967,295`  
4. `u64`, 对应 c++ `unsigned long long int` 类型.  
  数值范围在`0`~`18,446,744,073,709,551,615`  
5. `i8`, 对应 c++ `signed char` 类型.  
  数值范围在`-128`~`127`  
6. `i16`, 对应 c++ `signed short int` 类型.  
  数值范围在`-32,768`~`32,767`  
7. `i32`, 对应 c++ `signed long int` 类型.  
  数值范围在`-2,147,483,648`~`2,147,483,647`  
8. `i64`, 对应 c++ `signed long long int` 类型.  
  数值范围在`-9,223,372,036,854,775,808`~`9,223,372,036,854,775,807`  
9. `f32`, 对应 c++ `float` 类型.  
10. `f64`, 对应 c++ `double` 类型.  
11. `char`, 在 Windows 平台使用`wchar_t`类型.  
12. `str`, 字符串, 在 Windows 平台使用`std::wstring`类型.  
16. `bool`, 对应 c++ `bool` 类型.  

注意, PTDS 中类型不检查大小写, 即`i8` `bool` `char` `str`和`I8` `BOOL` `CHAR` `STR` `BoOl` `ChAr` `sTr`等都是正确的  

对应 PTDS.h 中`Petal::PTDSBasicTypeEnum`枚举类和`Petal::PTDSBasicType`命名空间中的`u8`, `u16`, `u32`, `u64`, `i8`, `i16`, `i32`, `i64`, `f32`, `f64`, `cha`, `str`, `bln`.  

### 元素数量

[实体](#实体)中元素的个数是实体类型的一部分,  
如`[array] { 1, 2, 3 }`中, 元素数量为 3.

### 指明类型

PTDS 内可以为实体中的值指定类型, 指定类型的方法有三种, 按优先顺序排序为  
1. [尖括号内前置指明](#前置指明类型)(使用尖括号`<>`指明值的类型和元素数量)  
2. [大括号后置指明](#大括号后置指明类型)(在实体的大括号后指明值的类型)  
3. [值后置指明](#值后置指明类型)(在值后指明值的类型)  

检查指明的类型时, 先检查前置指明, 再检查大括号后置指明, 最后按值的先后顺序检查值后置类型指明.  

指明类型时, 指明的类型不能与值本身[类型相悖](#类型相悖).  
使用多种方式指明类型时, 不能指明不一致的类型.  

#### 前置指明类型

在实体的前面, 标签的后面使用`<>`指明类型, 可以指明实体的元素数量和值的类型  
如`[array]<i32, 3> { 1, 2, 3 }` `[array2]<5, u8> 1, 2, 3, 4, 5 `  
也可以只指明元素数量和值的类型其中一方  
如`[array]<3> { 1, 2, 3 }` `[sign]<char> {"X"}`  

#### 后置指明类型时的类型别名

后置指明类型时部分类型名称可以进行简写(不区分大小写)  
| Type | Abbr | Abbr |
| :---: | :---: | :---: |  
| `i64` | `i` | `I` |  
| `u64` | `u` | `U` |  
| `f32` | `f` | `F` |  
| `char` | `c` | `C` |  
| `str` | `s` | `S` |  

在后置指明类型的时候, 不提供`bool`类型的指定方式, 请使用[自动类型推导](#自动类型推导)或[前置类型指明](#前置指明类型).  

#### 大括号后置指明类型

大括号后置指明类型只能指明值的类型  
如 `[array] { 1, 2, 3 }i32` `[array2]<2> { 1, 2 } u32` `[val] { 1.5 } f`

#### 值后置指明类型

在值的后面指明类型只能指明值的类型  
如`[array] 1i32, 2, 3 ` `[array] { 1, 2.f, 3 }` `[vals] 1, 2, 3u `  
注：设计之初并未考虑支持在十进制整数后使用`f`, `f32`, `f64`为其指明为浮点数类型如`1f`, 解析器的实现过程中没有禁止这种形式, 编写 ptds 文件时应当尽量避免这种形式.  

#### 类型相悖

类型相悖是指值不能被解解析为指定的类型,  
如  
1. 字符和字符串不能被解析为其他类型  
`[bool]<bool> { "true" } // 错误`  
2. 长度不为1的字符串不能被解析为字符  
`[char]<char> { "", "abc" } // 错误`
3. 浮点数不能被解析为整数  
`[array]<i32> { 1.5 } // 错误`  
4. 浮点数不能被解析为布尔值  
`[bool]<bool> { 1.1 }`
5. 负数不能指定为无符号整数类型  
`[array]<u32> { -1 } // 错误`  
6. 超过指定类型范围的值不能被解析为指定类型  
`[array]<i8> { -129 } // 错误`
`[array]<u8> { 256 } // 错误`
7. 布尔值不能被解析为非布尔值  
`[char]<char> { t }`

...

### 自动类型推导

PTDS 允许不指明实体类型而靠解析器自动推导  
PTDS 实体的类型在自动推导时根据实体的第一个元素确定  
推导规则:  
1. 可以由`i64`表示的整数, 将被推导为`i64`类型  
2. 超过`i64`表示范围但在`u64`范围内的正整数, 推导为`u64`, 无法用`i64`和`u64`表示的整数只能显式指定为浮点类型表示  
3. 带有小数点的浮点数始终被推导为`f64`, 而非`f32`  
4. `""`引用的内容始终被推导为`str`, 而非`char`  
5. `true` `t` `false` `f`将推导为`bool`  

示例
```
[test-int]    1, 2, 3 // 正确, 推导为'i64'类型  
[test-float]  1, 2.0, 3.0 // 错误, 根据推导规则, 取第一个元素'1'推导出的类型'i64', 而和第二个和第三个元素类型相悖  
[test-bool]   1, 0, true, false // 错误, 根据推导规则, 推导为'i64'类型, 而后面出现了只能被解析为'bool'类型的'true''false', 类型相悖  
[test-string] "c", "s", "" // 正确, 推导为'str'类型  
[test-double] 1., 2, 3, 4, 5 // 正确, 推导为'f64'类型  
```
