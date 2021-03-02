#pragma once
#include "synchronized.cpp"

#include <istream>
#include <ostream>
#include <vector>
#include <string>
#include <future>
#include <unordered_map>

class InvertedIndex {
public:
	struct Entry {
		size_t docid, hitcount;
	};
	void Add(std::string document);

	const std::vector<InvertedIndex::Entry>& Lookup(
			const std::string &word) const;

	void Swap(InvertedIndex &new_index, size_t &docs_count);

	const std::string& GetDocument(size_t id) const {
		return docs[id];
	}

	size_t GetDocsCount() const {
		return docs.size();
	}

	void PrintAll() const;
	void PrintIndex() const;
	void PrintDocuments() const;

private:
	std::unordered_map<std::string, std::vector<Entry>> index;
	std::vector<std::string> docs;
};

class SearchServer {
public:
	SearchServer() = default;
	explicit SearchServer(std::istream &document_input);
	void UpdateDocumentBase(std::istream &document_input);
	void UpdateDocumentBaseST(std::istream &document_input);
	void AddQueriesStream(std::istream &query_input,
			std::ostream &search_results_output);
	void AddQueriesStringST(std::vector<std::string> queries,
			std::ostream &search_results_output);

private:
	Synchronized<InvertedIndex> index;

	size_t docs_count = 0;
	std::vector<std::future<void>> futures;
};
