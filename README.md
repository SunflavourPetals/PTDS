# 仓库结构

`sln` 中为 ptds 解释器(ptds.vcxproj)和 demo(demo.vcxproj) 的 VS 解决方案  
`bin` 中有 `demo` 和 `ptds_lib` 两个文件夹  
`demo` 中为演示程序, 包含可执行文件、test.ptds文件和x64-release版本的动态链接库  
`ptds_lib` 中为 ptds 解释器的静态库、动态库、源文件和头文件  
`readme.md`  
`concepts.md` 详细的 PTDS 文件的编写规范  

# About PTDS
PTDS(Petal Textualized Data Store)
是自制的文本数据交换格式, 实现的解释器支持采用 utf-16 le 编码的文本文件.  
PTDS 使用八个标点符号`[]<>{}",`完成对数据的定义  
形如`[name]<type, size>{attr...}`  
```
[Title] <str, 1> { "Picnic" }
[tag] <str, 3> { "animal", "nature", "outdoor" }
```
其中 `<type, size>` 和`{}` 不是必要的, 类型和元素数量可以由 ptds 解释器自行推导  
```
[Title] "Picnic"
[tag]   "animal", "nature", "outdoor"
```
可以嵌套标签进行数据定义`[name]{[name]attr}`  
```
// ptds
[sth] {
    [pen] {
        [color] "red"
        [code]  1001
    }
}
```
在 c++ 程序查询时标签内的名称用`:`进行连接  
``` c++
void query_pen_color(const Petal::PTDS& ptds) {
    ptds.ElementStr(L"sth:pen:color");
}
```
详尽的规则请移步 [ptds-concepts](concepts.md "Standard of PTDS").  

# About ptds lib

# Load

PTDS 的相关内容都在 namespace `Petal` 中,  
若要打开磁盘上的 ptds 文件, 请使用 `PTDS::LoadPTDS` 方法,  
被打开的文件必须为带有 BOM 的 utf-16(le) 编码的文本文件,  
并且文件中的内容符合 ptds 的格式.  
对于 buffer 上的 ptds 内容,  
可以使用`PTDS::LoadPTDSFromBuffer`和`PTDS::LoadPTDSFromOuterBuffer`方法,  
前者会复制一遍缓冲区内的内容, 至载入并解释结束后释放,  
后者直接引用传入的缓冲区, 不会进行复制, 调用方只需保证在此方法结束前不要更改或释放这段缓冲区即可.  
使用这两种方法时, 缓冲区内不必带有 BOM.  

以上三个方法在失败时将抛出异常`Petal::PTDSException`,  
抛出异常即说明打开文件或解释失败,  
具体内容请参照`PTDS.h`头中的内容.  

# Query

在载入并解释成功后, ptds 文件中的内容会被解析称相应的格式, 并存在`PTDS::pto`引用的对象中,  
它是一个`std::unordered_map<PTDSBasicType::str, PTDSValueSet>`类型的对象,  
查询时可以调用的方法
1. `PTDS::Entity`查询实体, 要求提供实体名称, 成功则返回`const Petal::PTDSValueSet&`类型 const 对象.  
2. `PTDS::ElementXXX`查询实体中的值, 要求提供实体名称和索引(索引从0开始), 成功则返回相应类型的 const 值.  
3. `PTDS::PTDSObject`得到 PTDO const 对象, 使用其他方法查询.  

`PTDS::Entity`和`PTDS::ElementXXX`查询失败时, 会抛出异常`Petal::PTDSQueryException`,  
抛出异常说明查询失败, 没有拿到有效的值,  
具体内容请参照`PTDS.h`头中的内容.  

PTDS 文件中的数据为非字符串类型的数据时, `Petal::PTDSValueSet::v[n]::str`将记录文件中相应值的原始字符串,  
若为字符串类型的数据, `Petal::PTDSValueSet::str`中的内容就是字符串的内容(转义后),  
如  
```
[" PTDS "] [num] + (" record sign and value ") 1
           [hex] + 0xFFff00aA
           [str]   "string...\x20..."
```
则`Petal::PTDSValueSet::v[0]::str`中的内容分别为`+1`, `+0xFFff00aA`, `string... ...`   
在 C++ 程序中调用`Petal::PTDS::OriElemStr`查询原始的字符串或字符串类型的值.  
此方法失败时会抛出异常`Petal::PTDSQueryException`,  
抛出异常说明查询失败, 没有拿到有效的值.  

# About Demo

进入程序后先进行本地化配置使`std::wcout` `std::wcin`正常工作,  
若是简体中文环境, 则输入`chs`.  
若出现差错可能会使`std::wcout`无法输出正确的内容, `std::wcin`也将无法正确录入内容, 届时请重启程序, 并重新进行正确的 locale 设置.  

输入实体名称时, 请严丝合缝地输入, 如果有多余的任何字符如空格等, 都会导致找不到实体(只是个demo, 我懒得去处理这些了).  

在输入文件名和输入实体名称时可以输入`:`查看当前可用的命令  
实用的命令有
1. `:q` 退出
2. `:cls` `:clear` 清屏
3. `:all` 输出当前打开的.ptds文件中所有内容
4. `:reload` 重新加载当前打开的.ptds文件
