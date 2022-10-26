#include <iostream>
#include <fstream>
#include <vector>
#include <optional>
#include <string>
#include <list>
#include <queue>
#include <cstdio>
#include <random>
#include <chrono>
#include <execution>

class InputFileWithSortedSegments;
class OutputFileWithSortedSegments {
public:
	OutputFileWithSortedSegments(const std::string& file_name);
	void write(int x);
	InputFileWithSortedSegments getInStream();
	void removeFile() {
		fout.close();
		std::remove(m_file_name.c_str());
	}
	const std::string& name() const {
		return m_file_name;
	}

private:
	std::ofstream fout;
	std::string m_file_name;
};

class InputFileWithSortedSegments {
public:
	InputFileWithSortedSegments(const std::string& file_name) : fin(file_name), m_file_name(file_name) { fin >> prev_el; }
	int read() {
		is_end_of_segment = false;
		int x = prev_el;
		if (!eof())
			fin >> prev_el;
		if (x > prev_el)
			is_end_of_segment = true;
		return x;

	}
	bool eof() const {
		return fin.eof();
	}
	OutputFileWithSortedSegments getOutStream() {
		fin.close();
		return OutputFileWithSortedSegments(m_file_name);
	}
	bool IsEndOfSegment() const {
		return is_end_of_segment || eof();
	}
	void removeFile() {
		fin.close();
		std::remove(m_file_name.c_str());
	}
	const std::string& name() const {
		return m_file_name;
	}
	void close() {
		fin.close();
	}

private:
	std::ifstream fin;
	int prev_el;
	bool is_end_of_segment = false;
	std::string m_file_name;
};

OutputFileWithSortedSegments::OutputFileWithSortedSegments(const std::string& file_name) : fout(file_name), m_file_name(file_name) {

}
void OutputFileWithSortedSegments::write(int x) {
	fout << x << '\n';
}
InputFileWithSortedSegments OutputFileWithSortedSegments::getInStream() {
	fout.close();
	return InputFileWithSortedSegments(m_file_name);
}

struct ElementFromFile {
	int x;
	std::list<InputFileWithSortedSegments>::iterator it;
	bool operator>(const ElementFromFile& r) const {
		return x > r.x;
	}
};

// TODO
// Выделить сливание одного сегмента пёр файл
// Функция: расчелениние числа на пары фибоначчи
// Функция: сливание многофазной сортировкой: вход: инпут поток, кол-во сегментов в 1 файл и в 2 файл, выход аутпут поток
// Методы переименнования файла и удаления 

void FillFileBySegments(InputFileWithSortedSegments& input, int segments_cnt, OutputFileWithSortedSegments& output) {
	for (int i = 0; i < segments_cnt; i++) {
		do {
			output.write(input.read());
		} while (!input.IsEndOfSegment() && !input.eof());
	}
}

InputFileWithSortedSegments TwoPhaseSort(InputFileWithSortedSegments& input, std::pair<int, int> segments_cnt) {
	static int file_num = 1;
	OutputFileWithSortedSegments file1(std::string("__temp1_") + std::to_string(file_num)), file2(std::string("__temp2_") + std::to_string(file_num));
	FillFileBySegments(input, segments_cnt.first, file1);
	FillFileBySegments(input, segments_cnt.second, file2);

	std::list<InputFileWithSortedSegments> input_files;
	input_files.emplace_back(file1.getInStream());
	input_files.emplace_back(file2.getInStream());
	std::string file_name = std::string("__temp3_") + std::to_string(file_num++);
	OutputFileWithSortedSegments output_file(file_name);

	while (input_files.size() > 1) {
		std::priority_queue<ElementFromFile, std::vector<ElementFromFile>, std::greater<ElementFromFile>> last_el_per_file;
		std::optional<OutputFileWithSortedSegments> next_output_file;
		for (auto it = input_files.begin(); it != input_files.end(); it++) {
			last_el_per_file.push({ it->read(), it });
		}
		while (!last_el_per_file.empty()) {
			auto el_from_file = last_el_per_file.top();
			last_el_per_file.pop();

			output_file.write(el_from_file.x);

			if (!el_from_file.it->eof() && !el_from_file.it->IsEndOfSegment())
				last_el_per_file.push({ el_from_file.it->read(), el_from_file.it });
			else if (el_from_file.it->eof()) {
				if (!next_output_file)
					next_output_file = el_from_file.it->getOutStream();
				else
					el_from_file.it->removeFile();
				input_files.erase(el_from_file.it);
			}
		}
		if (next_output_file) {
			input_files.emplace_back(output_file.getInStream());
			output_file = std::move(*next_output_file);
			next_output_file = std::nullopt;


			std::clog << segments_cnt.second << std::endl;
			segments_cnt.second -= segments_cnt.first;
			std::swap(segments_cnt.first, segments_cnt.second);

		}
	}
	output_file.removeFile();
	return std::move(input_files.back());
}

std::vector<std::pair<int, int>> DivideNumberIntoFibPairs(int num) {
	std::vector<std::pair<int, int>> res;

	while (num > 0) {
		int fib_a = 0, fib_b = 1;
		while (fib_a + fib_b <= num) {
			fib_a += fib_b;
			std::swap(fib_a, fib_b);
		}
		res.emplace_back(fib_b - fib_a, fib_a);
		num -= fib_b;
	}

	return res;
}

int main()
{
	/*{
		std::mt19937 gen;
		std::ofstream fout("input.txt");
		for (int j = 0; j < 10; j++) {
			for (int i = 0; i < 1e7; i++)
				fout << gen() % 1'000'000'000 << '\n';
			std::clog << j * 10 << '%' << std::endl;
		}
	}
	std::clog << "Generating done" << std::endl;*/
	auto curtime = std::chrono::high_resolution_clock::now();

	InputFileWithSortedSegments raw_fin("input.txt");
	OutputFileWithSortedSegments sorted_out("__temp_sorted");
	int segments_count = 0;
	while (!raw_fin.eof()) {
		std::vector<int> v;
		v.reserve(5e7);
		for (int i = 0; i < 5e7 && !raw_fin.eof(); i++) {
			v.push_back(raw_fin.read());
		}
		segments_count++;
		std::sort(std::execution::par, v.begin(), v.end());
		for (int x : v)
			sorted_out.write(x);
		std::clog << "Segment was sorted" << std::endl;
	}
	std::clog << "Segments count is " << segments_count << std::endl;

	InputFileWithSortedSegments fin = sorted_out.getInStream();
	auto fib_numbers = DivideNumberIntoFibPairs(segments_count);
	std::list<InputFileWithSortedSegments> to_merge;
	for (auto fib_pair : fib_numbers) {
		to_merge.emplace_back(TwoPhaseSort(fin, fib_pair));
	}

	if (to_merge.size() > 1) {
		OutputFileWithSortedSegments output_file("output.txt");
		std::priority_queue<ElementFromFile, std::vector<ElementFromFile>, std::greater<ElementFromFile>> last_el_per_file;
		for (auto it = to_merge.begin(); it != to_merge.end(); it++) {
			last_el_per_file.push({ it->read(), it });
		}
		while (!last_el_per_file.empty()) {
			auto el_from_file = last_el_per_file.top();
			last_el_per_file.pop();

			output_file.write(el_from_file.x);

			if (!el_from_file.it->IsEndOfSegment())
				last_el_per_file.push({ el_from_file.it->read(), el_from_file.it });
			else {
				el_from_file.it->removeFile();
				to_merge.erase(el_from_file.it);
			}
		}
	}
	else {
		to_merge.back().close();
		std::rename(to_merge.back().name().c_str(), "output.txt");
	}
	fin.removeFile();
	auto total_seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - curtime).count();
	std::clog << "Total time is " << total_seconds << 's';
}