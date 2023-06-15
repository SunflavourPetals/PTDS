# 仓库结构

`sln` 中为 ptds 解释器(ptds.vcxproj)和 demo(demo.vcxproj) 的 VS 解决方案  
`bin` 中有 `demo` 和 `ptds_lib` 两个文件夹
`demo` 中为演示程序, 包含可执行文件、test.ptds文件和x64-release版本的动态链接库
`ptds_lib` 中为 ptds 解释器的静态库、动态库、源文件和头文件
`readme.md`
`concepts.md` 详细的 PTDS 文件的规范

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
在 c++ 程序查询时标签用`:`进行连接  
``` c++
void query_pen_color(const Petal::PTDS& ptds) {
    ptds.ElementStr(L"sth:pen:color");
}
```
详尽的规则请移步 [ptds-concepts](concepts.md "Standard of PTDS").  

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
