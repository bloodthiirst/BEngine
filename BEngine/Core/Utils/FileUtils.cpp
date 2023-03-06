#include "FileUtils.h"
#include <fstream>

std::vector<char> FileUtils::ReadFile ( const std::string& filePath )
{
	std::ifstream file ( filePath, std::ios::binary );

	if ( !file.is_open () )
		throw std::runtime_error ( "Error opnening the file" );

	size_t fileSize = static_cast<size_t>(file.tellg ());
	std::vector<char> fileData = std::vector<char> ( fileSize );

	file.seekg ( 0 );
	file.read ( fileData.data (), fileSize );

	return fileData;
}
