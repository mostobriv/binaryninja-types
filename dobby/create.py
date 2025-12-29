import binaryninja
from binaryninja.enums import NamedTypeReferenceClass
from binaryninja.types import Type, NamedTypeReferenceType, StructureBuilder, EnumerationBuilder

import os

supported_architectures = [
	"aarch64",
	"armv7",
]

for archname in supported_architectures[:1]:
	arch = binaryninja.Architecture[archname]
	typelib = binaryninja.typelibrary.TypeLibrary.new(arch, f"libdobby-{archname}.so")
	typelib.add_platform(binaryninja.Platform[f"mac-{archname}"])
	typelib.add_platform(binaryninja.Platform[f"ios-{archname}"])
	typelib.add_platform(binaryninja.Platform[f"linux-{archname}"])
	typelib.add_platform(binaryninja.Platform[f"windows-{archname}"])

	with open(os.path.join(os.path.dirname(__file__), "dobby.h"), "r") as f:
		my_types = binaryninja.TypeParser.default.parse_types_from_source(
			f.read(),
			"dobby.h",
			binaryninja.Platform[f"linux-{archname}"],
			options=[
				"-x",
				"c++",
				"-std=c++20",
				# "-I",
				# "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/17/include",
			],
		)

		print(my_types)

		# with open(os.path.join(os.path.dirname(__file__), "dobby.h"), "r") as f:
		# 	t = binaryninja.TypeParser.default.parse_type_string(f.read(), binaryninja.Platform["aarch64"])
		# 	print(t)

		result, error = binaryninja.TypeParser.default.parse_type_string(
			"int DobbyCodePatch(void *address, uint8_t *buffer, uint32_t buffer_size);",
			platform=bv.platform,
			existing_types=bv,
		)

		name, t = result

		print(name, t)

		typelib.add_named_object(name, t)

		typelib.finalize()
		typelib.write_to_file(
			os.path.join(binaryninja.user_directory(), "typelib", f"dobby-{archname}.bntl")
		)
