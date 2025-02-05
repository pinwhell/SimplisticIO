#include <iostream>
#include <simplistic/io/SelfIO.h>
//#include <simplistic/io/Object.h>
#include "../include/simplistic/io/Object.h"

using namespace simplistic::io;

int main()
{
    int x = 10;
	std::string example = "ThisIsAnAmazingExample - Lorem ipsum odor amet, consectetuer adipiscing elit. Nascetur taciti lacinia turpis rutrum ligula consectetur suscipit per magnis. Non aliquam litora parturient ullamcorper potenti pellentesque. Cras ipsum suspendisse nunc maecenas proin lobortis scelerisque urna. Ipsum gravida imperdiet cras; suscipit ex nascetur nulla praesent netus. Bibendum accumsan tempor sodales phasellus, dictum molestie tortor. Ante consectetur tortor bibendum nullam varius. Viverra turpis ornare at taciti mauris fames mus mus.";
	const char* pExample = example.data();
	Self io{};
	Object obj(&io, 0);
	obj += 0;
	
	auto y = obj.Read<int>(&x);
	std::string s1 = *obj.Address<OString>(example.data());
	std::string s2 = *obj.Derref<OString>(&pExample);
	auto s3 = obj
		.Address(&pExample)
		.Derref<OString>()
		.Read(); 
	std::cout << std::boolalpha << (
		example == s1 &&
		example == s2 &&
		example == s3 &&
		obj.Derref(&pExample).mEntry == (std::uint64_t)pExample &&
		obj.Derref(&pExample).Read<char>() == 'T' &&
		*(std::uint32_t*)(obj.Derref(&pExample).ReadBlob(4).data()) == *(std::uint32_t*)example.data()
	);
    return 0;
}