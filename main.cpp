/*
 * Copyright (C) 2017 koolkdev
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <cstdio>
#include <vector>
#include <fstream>
#include <memory>
#include <iostream>
#include <sstream>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <wfslib/WfsLib.h>

#include "games_list.h"

template<typename T>
void enter(const std::string& msg, T& t)
{
	std::cout << msg;
	std::string str;
	std::getline(std::cin, str);
	std::istringstream iss(str);
	iss >> t;
}

int main(int argc, char *argv[]) {
	try {
		boost::program_options::options_description desc("Allowed options");
		std::string wfs_path;
		desc.add_options()
			("help", "produce help message")
			("image", boost::program_options::value<std::string>(), "wfs image file")
			("otp", boost::program_options::value<std::string>(), "otp file")
			("seeprom", boost::program_options::value<std::string>(), "seeprom file (required if usb)")
			("mlc", "device is mlc (default: device is usb)")
			("usb", "device is usb")
			;

		boost::program_options::variables_map vm;
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
		boost::program_options::notify(vm);

		bool bad = false;
		if (!vm.count("image")) { std::cerr << "Missing wfs image file (--image)" << std::endl; bad = true; }
		if (!vm.count("otp")) { std::cerr << "Missing otp file (--otp)" << std::endl; bad = true; }
		if ((!vm.count("seeprom") && !vm.count("mlc"))) { std::cerr << "Missing seeprom file (--seeprom)" << std::endl; bad = true; }
		if (vm.count("mlc") + vm.count("usb") > 1) { std::cerr << "Can't specify both --mlc and --usb" << std::endl; bad = true; }
		if (vm.count("help") || bad) {
			std::cout << "Usage: wfs-file-injector --image <wfs image> [--tid <ds game tid>] -otp <opt path> [--seeprom <seeprom path>] [--mlc] [--usb]" << std::endl;
			std::cout << desc << "\n";
			return 1;
		}

		std::vector<uint8_t> key;
		std::unique_ptr<OTP> otp;
		// open otp
		try {
			otp.reset(OTP::LoadFromFile(vm["otp"].as<std::string>()));
		}
		catch (std::exception& e) {
			std::cerr << "Failed to open OTP: " << e.what() << std::endl;
			return 1;
		}

		if (vm.count("mlc")) {
			// mlc
			key = otp->GetMLCKey();
		}
		else {
			// usb
			std::unique_ptr<SEEPROM> seeprom;
			try {
				seeprom.reset(SEEPROM::LoadFromFile(vm["seeprom"].as<std::string>()));
			}
			catch (std::exception& e) {
				std::cerr << "Failed to open SEEPROM: " << e.what() << std::endl;
				return 1;
			}
			key = seeprom->GetUSBKey(*otp);
		}

		auto device = std::make_shared<FileDevice>(vm["image"].as<std::string>(), 9, false);
		Wfs::DetectSectorsCount(device, key);
		Wfs::DetectSectorSize(device, key);
		Wfs wfs(device, key);
		std::vector<GameInfo> detected_games;
		for (auto game : GamesList) {
			if (wfs.GetFile((boost::format("/usr/title/00050000/%08x/content/0010/rom.zip") % game.tid).str())) {
				detected_games.push_back(game);
			}
		}
		GameInfo selected_game;

		if (detected_games.empty()) {
			std::cerr << "Error: Didn't find any DS compatible game." << std::endl;
			return 1;
		}
		else if (detected_games.size() > 1) {
			std::cout << "Detected multiple games:" << std::endl;
			for (unsigned int i = 0; i < detected_games.size(); ++i) {
				std::cout << (boost::format("%d: Game: %s, TID: 0x%08x") % (i+1) % detected_games[i].name % detected_games[i].tid) << std::endl;
			}
			unsigned int choice = 0;
			enter((boost::format("Select a game: [1-%d] ") % detected_games.size()).str(), choice);
			if (choice >= 1 && choice <= detected_games.size()) {
				selected_game = detected_games[choice-1];
			}
			else {
				std::cout << "Invalid choice.";
				return 1;
			}
		}
		else {
			std::cout << (boost::format("Detected Game: %s, TID: 0x%08x") % detected_games[0].name % detected_games[0].tid) << std::endl;
			char choice = 'n';
			enter("Is it correct? [y/n] ", choice);
			if (choice != 'Y' && choice != 'y') {
				std::cout << "Bye!";
				return 1;
			}
			selected_game = detected_games[0];
		}
		std::ifstream input_file((boost::filesystem::path("data") / selected_game.filename).string(), std::ios::binary | std::ios::in);
		if (input_file.fail()) {
			std::cerr << "Failed to open rom " << selected_game.filename << std::endl;
			return 1;
		}
		input_file.seekg(0, std::ios::end);
		if (static_cast<uint64_t>(input_file.tellg()) > SIZE_MAX) {
			std::cerr << "Error: File to inject too big" << std::endl;
			return 1;
		}
		size_t file_size = static_cast<size_t>(input_file.tellg());
		input_file.seekg(0, std::ios::beg);
		auto file = wfs.GetFile((boost::format("/usr/title/00050000/%08x/content/0010/rom.zip") % selected_game.tid).str());
		if (!file) {
			std::cerr << "Unexepcted error: Failed to open rom in wfs image" << std::endl;
			return 1;
		}
		if (file_size > file->GetSizeOnDisk()) {
			std::cerr << "Error: Rom too big (wanted size: " << file_size << " bytes, available size: " << file->GetSizeOnDisk() << ")" << std::endl;
			return 1;
		}
		File::stream stream(file);
		std::vector<char> data(0x2000);
		size_t to_copy = file_size;
		while (to_copy > 0) {
			input_file.read((char*)&*data.begin(), std::min(data.size(), to_copy));
			auto read = input_file.gcount();
			if (read <= 0) {
				std::cerr << "Error: Failed to read rom" << std::endl;
				return 1;
			}
			stream.write(&*data.begin(), read);
			to_copy -= static_cast<size_t>(read);
		}
		input_file.close();
		stream.close();
		if (file_size < file->GetSize()) {
			file->Resize(file_size);
		}
		std::cout << "Done!" << std::endl;
	}
	catch (std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}