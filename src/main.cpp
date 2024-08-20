#include "util.h"
#include "yaz0.h"

#include <cstdio>
#include <fstream>

#include <filesystem>
namespace fs = std::filesystem;

enum class Format {
	Yaz0,
	SARC,
	SZS,
};

enum class Option {
	Read,
	Write,
};

s32 main(s32 argc, char* argv[]) {
	if (argc < 3) {
		fprintf(stderr, "usage: %s <format> <option>\n", argv[0]);
		fprintf(stderr, "\tformats: yaz0, sarc, szs\n");
		fprintf(stderr, "\toptions: read, r, write, w\n");
		return 1;
	}
	
	Format format;
	if (util::isEqual(argv[1], "yaz0"))
		format = Format::Yaz0;
	else if (util::isEqual(argv[1], "sarc"))
		format = Format::SARC;
	else if (util::isEqual(argv[1], "szs"))
		format = Format::SZS;
	else {
		fprintf(stderr, "error: unrecognized format '%s'\n", argv[1]);
		return 1;
	}

	Option option;
	if (util::isEqual(argv[2], "read") || util::isEqual(argv[2], "r"))
		option = Option::Read;
	else if (util::isEqual(argv[2], "write") || util::isEqual(argv[2], "w"))
		option = Option::Write;
	else {
		fprintf(stderr, "error: unrecognized option '%s'\n", argv[2]);
		return 1;
	}

	if (false) {
		std::string base_path = "/home/tetra/nx/smo/100/romfs/";
		for (const auto& dir : fs::directory_iterator(base_path)) {
			const char* dir_name = dir.path().filename().c_str();
			for (const auto& file_path : fs::directory_iterator(dir)) {
				if (!file_path.path().string().ends_with(".szs")) continue;
				const char* file_name = file_path.path().filename().c_str();
				printf("%s\n", file_path.path().c_str());

				printf("read\n");
				std::vector<u8> contents;
				s32 r = util::readFile(file_path.path().c_str(), contents);
				if (r != 0) return r;
				u32 comp_size = contents.size();

				printf("decomp\n");
				std::vector<u8> decompressed;
				r = yaz0::decompress(contents, decompressed);
				if (r != 0) return r;
				u32 decomp_size = decompressed.size();

				printf("recomp\n");
				std::vector<u8> recompressed;
				yaz0::compress(decompressed, recompressed, 0x80);
				u32 recomp_size = recompressed.size();

				f32 orig_ratio = (f32)comp_size / (f32)decomp_size;
				f32 my_ratio = (f32)recomp_size / (f32)decomp_size;

				printf("%12x %12x %12x %.3f %.3f %s/%s\n", comp_size, decomp_size, recomp_size, orig_ratio, my_ratio, dir_name, file_name);
			}
		}


		return 0;
	}



	switch (format) {
	case Format::Yaz0:
		if (option == Option::Read) {
			if (argc < 5) {
				fprintf(stderr, "usage: %s yaz0 r <compressed file> <decompressed file>\n", argv[0]);
				return 1;
			}

			std::vector<u8> fileContents;
			s32 r = util::readFile(argv[3], fileContents);
			if (r != 0) return r;

			std::vector<u8> outputBuffer;
			r = yaz0::decompress(fileContents, outputBuffer);
			if (r != 0) return r;

			std::ofstream outfile(argv[4], std::ios::out | std::ios::binary);
			outfile.write(reinterpret_cast<const char*>(outputBuffer.data()), outputBuffer.size());
		}
		else {
			if (argc < 5) {
				fprintf(stderr, "usage: %s yaz0 w <decompressed file> <compressed file> [alignment]\n", argv[0]);
				return 1;
			}

			u32 alignment = argc > 5 ? atoi(argv[5]) : 0x80;

			std::vector<u8> fileContents;
			s32 r = util::readFile(argv[3], fileContents);
			if (r != 0) return r;

			std::vector<u8> outputBuffer;
			yaz0::compress(fileContents, outputBuffer, alignment);

			std::ofstream outfile(argv[4], std::ios::out | std::ios::binary);
			outfile.write(reinterpret_cast<const char*>(outputBuffer.data()), outputBuffer.size());
		}


		break;
	case Format::SARC:
		if (argc < 4) {
			fprintf(stderr, "usage: %s sarc <archive> <output dir>\n", argv[0]);
			return 1;
		}

		break;
	case Format::SZS:
		if (argc < 4) {
			fprintf(stderr, "usage: %s szs <archive> <output dir>\n", argv[0]);
			return 1;
		}

		break;
	}

	return 0;
}
