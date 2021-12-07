#include <string>
#include <iostream>

namespace Permute {

std::string gen_brute_str(const size_t length, size_t i = 0);

std::string gen_brute_str(const size_t length, size_t i) {
	const static std::string alphanum{"0123456789abcdefghijklmnopqrstuvwxyz"};
	static std::string genString(length, alphanum[0]);
	const static size_t alphanumLength = alphanum.length();

	if(i > length) {
		return genString;
	}

	if(genString[i] != alphanum[alphanumLength-1]) {
		genString[i] = alphanum[alphanum.find(genString[i])+1];
	}
	else {
		gen_brute_str(length, i+1);
		genString[i] = alphanum[(alphanum.find(genString[i])+1) % alphanumLength];
	}

	return genString;
}	

}
