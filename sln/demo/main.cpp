#include <PTDS.h>
#pragma comment(lib, "ptds.lib")

#include <iostream>
#include <string>

#include <Petal~PerformanceCounter.h>
// if Petal~PerformanceCounter.h not exist
// undef _Petal_PC
#define _Petal_PC

void print_entity_info(const Petal::PTDSValueSet& entity)
{
	using namespace Petal;
	using namespace std;
	switch (entity.t)
	{
	case PTDSBasicTypeEnum::i8:
		cout << "* Type: i8  -Size: " << entity.v.size() << endl;
		break;
	case PTDSBasicTypeEnum::i16:
		cout << "* Type: i16  -Size: " << entity.v.size() << endl;
		break;
	case PTDSBasicTypeEnum::i32:
		cout << "* Type: i32  -Size: " << entity.v.size() << endl;
		break;
	case PTDSBasicTypeEnum::i64:
		cout << "* Type: i64  -Size: " << entity.v.size() << endl;
		break;
	case PTDSBasicTypeEnum::u8:
		cout << "* Type: u8  -Size: " << entity.v.size() << endl;
		break;
	case PTDSBasicTypeEnum::u16:
		cout << "* Type: u16  -Size: " << entity.v.size() << endl;
		break;
	case PTDSBasicTypeEnum::u32:
		cout << "* Type: u32  -Size: " << entity.v.size() << endl;
		break;
	case PTDSBasicTypeEnum::u64:
		cout << "* Type: u64  -Size: " << entity.v.size() << endl;
		break;
	case PTDSBasicTypeEnum::f32:
		cout << "* Type: f32  -Size: " << entity.v.size() << endl;
		break;
	case PTDSBasicTypeEnum::f64:
		cout << "* Type: f64  -Size: " << entity.v.size() << endl;
		break;
	case PTDSBasicTypeEnum::bln:
		cout << "* Type: bool  -Size: " << entity.v.size() << endl;
		break;
	case PTDSBasicTypeEnum::cha:
		cout << "* Type: char  -Size: " << entity.v.size() << endl;
		break;
	case PTDSBasicTypeEnum::str:
		cout << "* Type: str  -Size: " << entity.v.size() << endl;
		break;
	default:
		cout << "! 类型错误" << endl;
		break;
	}
}
void print_entity_value(const Petal::PTDSValueSet& entity, const Petal::PTDSBasicType::tsize& index)
{
	using namespace Petal;
	using namespace std;
	if (entity.v.size() <= index)
	{
		cout << "! 数组越界" << endl;
		return;
	}
	switch (entity.t)
	{
	case PTDSBasicTypeEnum::i8:
		cout << entity.v[index].i8 << endl;
		break;
	case PTDSBasicTypeEnum::i16:
		cout << entity.v[index].i16 << endl;
		break;
	case PTDSBasicTypeEnum::i32:
		cout << entity.v[index].i32 << endl;
		break;
	case PTDSBasicTypeEnum::i64:
		cout << entity.v[index].i64 << endl;
		break;
	case PTDSBasicTypeEnum::u8:
		cout << entity.v[index].u8 << endl;
		break;
	case PTDSBasicTypeEnum::u16:
		cout << entity.v[index].u16 << endl;
		break;
	case PTDSBasicTypeEnum::u32:
		cout << entity.v[index].u32 << endl;
		break;
	case PTDSBasicTypeEnum::u64:
		cout << entity.v[index].u64 << endl;
		break;
	case PTDSBasicTypeEnum::f32:
		cout << entity.v[index].f32 << endl;
		break;
	case PTDSBasicTypeEnum::f64:
		cout << entity.v[index].f64 << endl;
		break;
	case PTDSBasicTypeEnum::bln:
		if (entity.v[index].bln == true) cout << "true" << endl;
		else cout << "false" << endl;
		break;
	case PTDSBasicTypeEnum::cha:
		wcout << entity.v[index].cha << endl;
		break;
	case PTDSBasicTypeEnum::str:
		wcout << entity.v[index].str << endl;
		break;
	default:
		cout << "! 类型错误" << endl;
		break;
	}
}
void print_all(const Petal::PTDO& o)
{
	using namespace Petal;
	using namespace std;
	for (const auto& e : o)
	{
		cout << "[E] ";
		wcout << e.first;
		cout << "  ";
		print_entity_info(e.second);
		for (Petal::PTDSBasicType::tsize i = 0; i < e.second.v.size(); ++i)
		{
			cout << "    [" << i + 1 << "] ";
			print_entity_value(e.second, i);
		}
	}
}
int wmain(int argc, wchar_t* argv[])
{
	using namespace Petal;
	using namespace std;

	PTDS ptds;
	string locale_str;
	WString file;
	WString tag;
	int index{ 0 };

	if (argc >= 2)
	{
		char* buff = new char[128];
		sprintf_s(buff, 128, "%ls", argv[1]);
		locale_str = buff;
		delete[] buff;
		try
		{
			wcout.imbue(std::locale(locale_str));
			wcin.imbue(std::locale(locale_str));
		}
		catch (std::exception&)
		{
			std::cout << "! Failed" << std::endl;
			goto _ptds_demo_Input_locale;
		}
	}
	else
	{
		_ptds_demo_Input_locale:
		for (;;)
		{
			cout << "* 设置 locale, 中文环境请输入“chs”" << endl << "* set locale: " << endl << ">>> ";
			cin >> locale_str;
			if (locale_str == ":q")
			{
				cout << "* 程序已退出" << endl;
				system("pause");
				return 0;
			}
			if (locale_str == ":cls" || locale_str == ":clear")
			{
				system("cls");
				continue;
			}
			try
			{
				wcout.imbue(std::locale(locale_str));
				wcin.imbue(std::locale(locale_str));
				break;
			}
			catch (std::exception&)
			{
				std::cout << "! Failed" << std::endl;
				continue;
			}
		}
	}
	if (argc >= 3)
	{
		file = argv[2];
		goto _ptds_demo_An_file_name;
	}
	for (;;)
	{
		cout << "* 输入要打开的文件名" << endl << ">>> ";
		wcin >> file;
		_ptds_demo_An_file_name:
		if (file == L":" || file == L":h")
		{
			cout
				<< "* \":q\" - quit" << endl
				<< "* \":cls\" / \":clear\" - clear screen" << endl;
			continue;
		}
		if (file == L":q")
		{
			cout << "* 程序已退出" << endl;
			system("pause");
			break;
		}
		if (file == L":cls" || file == L":clear")
		{
			system("cls");
			continue;
		}
		try
		{
#ifdef _Petal_PC
			PerformanceCounter f;
			f.Count();
			ptds.LoadPTDS(file.c_str());
			f.Count();
			cout << "* res time: " << f.DeltaTime() << " second" << endl;
#else
			ptds.LoadPTDS(file.c_str());
#endif
		}
		catch (Petal::PTDSException& e)
		{
			using namespace std;
			wcout
				<< L"! line: " << e.line << L" position: " << e.position
				<< L" error: " << static_cast<unsigned long long>(e.error_code)
				<< L" errc: " << e.character
				<< L" msg: " << e.message << endl;
			continue;
		}
		cout << "* 打开成功" << endl;

		for (;;)
		{
			cout << "* 请输入实体名称" << endl << ">>> ";
			wcin >> tag;
			if (tag == L":")
			{
				cout
					<< "* \":q\" - quit" << endl
					<< "* \":cls\" / \":clear\" - clear screen" << endl
					<< "* \":all\" - print all" << endl
					<< "* \":reload\" - reload file \"";
				wcout << file;
				cout << "\"" << endl;
				continue;
			}
			if (tag == L":q")
			{
				break;
			}
			if (tag == L":cls" || tag == L"clear")
			{
				system("cls");
				continue;
			}
			if (tag == L":all")
			{
				print_all(ptds.PTDSObject());
				continue;
			}
			if (tag == L":reload")
			{
				try
				{
#ifdef _Petal_PC
					PerformanceCounter f;
					f.Count();
					ptds.LoadPTDS(file.c_str());
					f.Count();
					cout << "* res time: " << f.DeltaTime() << " second" << endl;
#else
					ptds.LoadPTDS(file.c_str());
#endif
					cout << "* 重新加载成功" << endl;
					continue;
				}
				catch (Petal::PTDSException& e)
				{
					cout << "! 重新加载失败" << endl;
					using namespace std;
					std::wcout.imbue(std::locale("chs"));
					cout
						<< "! line: " << e.line << " position: " << e.position
						<< " error: " << static_cast<unsigned long long>(e.error_code)
						<< " errc: " << e.character
						<< " msg: ";
					wcout << e.message;
					cout << endl;
					break;
				}
			}
			try
			{
				const auto& entity = ptds.Entity(tag);

				print_entity_info(entity);

				if (entity.v.size() > 1)
				{
					cout << "* 请输入索引(0~" << entity.v.size() - 1 << ")" << endl << ">>> ";
					cin >> index;
				}
				else
				{
					index = 0;
				}

				if (entity.v.size() <= index)
				{
					cout << "! 数组越界" << endl;
					continue;
				}
				else
				{
					print_entity_value(entity, index);
				}
			}
			catch (Petal::PTDSQueryException&)
			{
				cout << "! 找不到实体" << endl;
			}
		}
	}
	return 0;
}
