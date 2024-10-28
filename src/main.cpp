#include <cstdio>
#include <fstream>

#include "bffnt.h"
#include "bntx.h"
#include "byml/reader.h"
#include "byml/writer.h"
#include "sarc.h"
#include "util.h"
#include "yaz0.h"

enum class Format {
	Yaz0,
	SARC,
	SZS,
	BFFNT,
	BNTX,
	BYML,
};

enum class Option {
	Read,
	Write,
};

s32 main(s32 argc, char* argv[]) {
	if (argc < 3) {
		fprintf(stderr, "usage: %s <format> <option>\n", argv[0]);
		fprintf(stderr, "\tformats: yaz0, sarc, szs, bffnt, bntx, byml\n");
		fprintf(stderr, "\toptions: read, r, write, w\n");
		return 1;
	}

	Format format;
	if (util::is_equal(argv[1], "yaz0"))
		format = Format::Yaz0;
	else if (util::is_equal(argv[1], "sarc"))
		format = Format::SARC;
	else if (util::is_equal(argv[1], "szs"))
		format = Format::SZS;
	else if (util::is_equal(argv[1], "bffnt"))
		format = Format::BFFNT;
	else if (util::is_equal(argv[1], "bntx"))
		format = Format::BNTX;
	else if (util::is_equal(argv[1], "byml"))
		format = Format::BYML;
	else {
		fprintf(stderr, "error: unrecognized format '%s'\n", argv[1]);
		return 1;
	}

	Option option;
	if (util::is_equal(argv[2], "read") || util::is_equal(argv[2], "r"))
		option = Option::Read;
	else if (util::is_equal(argv[2], "write") || util::is_equal(argv[2], "w"))
		option = Option::Write;
	else {
		fprintf(stderr, "error: unrecognized option '%s'\n", argv[2]);
		return 1;
	}

	// if (false) {
	// 	fs::path base_path = "/home/tetra/nx/smo/100/romfs/";
	// 	for (const auto& dir : fs::directory_iterator(base_path)) {
	// 		fs::path dir_name = dir.path().filename();
	// 		for (const fs::path& file_path : fs::directory_iterator(dir)) {
	// 			if (!file_path.string().ends_with(".szs")) continue;
	// 			const char* file_name = file_path.filename().c_str();
	// 			printf("%s\n", file_path.c_str());
	//
	// 			printf("read\n");
	// 			std::vector<u8> contents;
	// 			s32 r = util::read_file(file_path, contents);
	// 			if (r != 0) return r;
	// 			u32 comp_size = contents.size();
	//
	// 			printf("decomp\n");
	// 			std::vector<u8> decompressed;
	// 			r = yaz0::decompress(contents, decompressed);
	// 			if (r != 0) return r;
	// 			u32 decomp_size = decompressed.size();
	//
	// 			printf("recomp\n");
	// 			std::vector<u8> recompressed;
	// 			yaz0::compress(decompressed, recompressed, 0x80);
	// 			u32 recomp_size = recompressed.size();
	//
	// 			f32 orig_ratio = (f32)comp_size / (f32)decomp_size;
	// 			f32 my_ratio = (f32)recomp_size / (f32)decomp_size;
	//
	// 			printf("%12x %12x %12x %.3f %.3f %s/%s\n", comp_size,
	// decomp_size, recomp_size, orig_ratio, my_ratio, dir_name, file_name);
	// 		}
	// 	}
	//
	//     return 0;
	// }

	result_t r;

	switch (format) {
	case Format::Yaz0:
		if (option == Option::Read) {
			if (argc < 5) {
				fprintf(
					stderr, "usage: %s yaz0 r <compressed file> <decompressed file>\n", argv[0]
				);
				return 1;
			}

			std::vector<u8> fileContents;
			r = util::read_file(argv[3], fileContents);
			if (r) break;

			std::vector<u8> outputBuffer;
			r = yaz0::decompress(fileContents, outputBuffer);
			if (r) break;

			std::ofstream outfile(argv[4], std::ios::out | std::ios::binary);
			outfile.write(reinterpret_cast<const char*>(outputBuffer.data()), outputBuffer.size());
		} else {
			if (argc < 5) {
				fprintf(
					stderr,
					"usage: %s yaz0 w <decompressed file> <compressed file>"
					"[alignment]\n",
					argv[0]
				);
				return 1;
			}

			u32 alignment = argc > 5 ? atoi(argv[5]) : 0x80;

			std::vector<u8> fileContents;
			r = util::read_file(argv[3], fileContents);
			if (r) break;

			std::vector<u8> outputBuffer;
			yaz0::compress(fileContents, outputBuffer, alignment);

			util::write_file(argv[4], outputBuffer);
		}

		break;
	case Format::SARC:
		if (option == Option::Read) {
			if (argc < 5) {
				fprintf(stderr, "usage: %s sarc r <archive> <output dir>\n", argv[0]);
				return 1;
			}

			std::vector<u8> fileContents;
			r = util::read_file(argv[3], fileContents);
			if (r) break;

			SARC sarc(fileContents);
			r = sarc.read();
			if (r) break;

			r = sarc.save(argv[4]);
			if (r) break;
		}

		break;
	case Format::SZS:
		if (option == Option::Read) {
			if (argc < 5) {
				fprintf(stderr, "usage: %s szs r <archive> <output dir>\n", argv[0]);
				return 1;
			}

			std::vector<u8> fileContents;
			r = util::read_file(argv[3], fileContents);
			if (r) break;

			std::vector<u8> decompressed;
			r = yaz0::decompress(fileContents, decompressed);
			if (r) break;

			SARC sarc(decompressed);
			r = sarc.read();
			if (r) break;

			r = sarc.save(argv[4]);
			if (r) break;
		}

		break;
	case Format::BFFNT:
		if (option == Option::Read) {
			if (argc < 4) {
				fprintf(stderr, "usage: %s bffnt r <font file>\n", argv[0]);
				return 1;
			}

			std::vector<u8> fileContents;
			r = util::read_file(argv[3], fileContents);
			if (r) break;

			BFFNT bffnt(fileContents);
			r = bffnt.read();
			if (r) break;
		}

		break;
	case Format::BNTX:
		if (option == Option::Read) {
			if (argc < 4) {
				fprintf(stderr, "usage: %s bntx r <texture file>\n", argv[0]);
				return 1;
			}

			std::vector<u8> fileContents;
			r = util::read_file(argv[3], fileContents);
			if (r) break;

			BNTX bntx(fileContents);
			r = bntx.read();
			if (r) break;
		}

		break;
	case Format::BYML:
		if (option == Option::Read) {
			if (argc < 4) {
				fprintf(stderr, "usage: %s byml r <input file>\n", argv[0]);
				return 1;
			}

			std::vector<u8> fileContents;
			r = util::read_file(argv[3], fileContents);
			if (r) break;

			byml::Reader byml;
			r = byml.init(&fileContents[0]);
			if (r) break;

			printf("\nroot type: %x\n", (u8)byml.get_type());
			printf("root size: %d\n", byml.get_size());

			for (s32 scenarioIdx = 0; scenarioIdx < byml.get_size(); scenarioIdx++) {
				byml::Reader scenario;
				r = byml.get_container_by_idx(&scenario, scenarioIdx);
				if (r) break;

				printf("\tscenario type: %x\n", (u8)scenario.get_type());
				printf("\tscenario size: %d\n", scenario.get_size());

				if (!scenario.has_key("ObjectList")) {
					printf("\n");
					continue;
				}

				byml::Reader objectList;
				r = scenario.get_container_by_key(&objectList, "ObjectList");
				if (r) break;

				printf("\t\tobject list type: %x\n", (u8)objectList.get_type());
				printf("\t\tobject list size: %d\n", objectList.get_size());

				for (s32 objectIdx = 0; objectIdx < objectList.get_size(); objectIdx++) {
					byml::Reader object;
					r = objectList.get_container_by_idx(&object, objectIdx);
					if (r) break;

					std::string name;
					r = object.get_string_by_key(&name, "UnitConfigName");
					if (r) break;

					printf("\t\t\tobject name: %s\n", name.c_str());
				}
				if (r) break;

				printf("\n");
			}
			if (r) break;
		} else {
			if (argc < 4) {
				fprintf(stderr, "usage: %s byml w <output file>\n", argv[0]);
				return 1;
			}

			byml::Writer byml(3);
			byml.init();

			byml.push_array();
			byml.write_u32(123);
			byml.write_f32(1.0f);
			byml.write_s64(-1);
			byml.write_string("hihi");

			byml.push_array();
			byml.write_bool(true);
			byml.push_array();
			byml.write_u32(1);
			byml.pop();
			byml.pop();

			byml.push_array();
			byml.write_bool(true);
			byml.push_array();
			byml.write_u32(2);
			byml.pop();
			byml.pop();

			byml.push_hash();
			byml.write_u32("a", 100);
			byml.write_u32("b", 200);
			byml.push_array("xyz");
			byml.write_u32(123);
			byml.pop();
			byml.pop();

			byml.pop();

			// TODO: compression ratio for saving. can reuse identical container nodes/special type
			// nodes
			byml.save(argv[3], util::ByteOrder::Little);
		}

		break;
	}

	if (r) fprintf(stderr, "error %x: %s\n", r, result_to_string(r));

	return 0;
}
