#pragma once
#include <vector>
#include <string>

namespace Slyvina {
	namespace Azor {
		typedef void (*Azor_Command)(std::vector<std::string>);

		void RegCommand(std::string,Azor_Command);
	}
}
