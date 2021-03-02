#include "search_server.h"
#include "iterator_range.h"
#include "profile.h"
#include "paginator.cpp"

#include <algorithm>
#include <iterator>
#include <sstream>
#include <iostream>
#include <utility>
#include <deque>
#include <cmath>

using namespace std;

vector<string> SplitIntoWords(const string &line) {
	istringstream words_input(line);
	return {istream_iterator<string>(words_input), istream_iterator<string>()};
}

SearchServer::SearchServer(istream &document_input) {
	UpdateDocumentBaseST(document_input);
}

void SearchServer::UpdateDocumentBaseST(istream &document_input) {
	InvertedIndex new_index;
	for (string current_document; getline(document_input, current_document);) {
		new_index.Add(move(current_document));
	}
	index.GetAccess().ref_to_value.Swap(new_index, docs_count);
}

void SearchServer::UpdateDocumentBase(istream &document_input) {
	if (docs_count == 0) {
		UpdateDocumentBaseST(document_input);
	} else {
		futures.push_back(async([&document_input, this] {
			UpdateDocumentBaseST(document_input);
		}));
	}
//  InvertedIndex new_index;
//  for (string current_document; getline(document_input, current_document); ) {
//    new_index.Add(move(current_document));
//  }
//  index.GetAccess().ref_to_value.Swap(new_index);
}

void SearchServer::AddQueriesStream(istream &query_input,
		ostream &search_results_output) {
	vector<string> queries;
	for (string current_query; getline(query_input, current_query);) {
		queries.push_back(move(current_query));
	}
	futures.push_back(async([queries, &search_results_output, this] {
		AddQueriesStringST(move(queries), search_results_output);
	}));

//  vector<pair<size_t, size_t>> search_results;
//  vector<pair<size_t, bool>> docids_positions(index.GetAccess().ref_to_value.DocsSize());
//  for (string current_query; getline(query_input, current_query); ) {
//	const auto words = SplitIntoWords(current_query);
//	search_results.clear();
//	docids_positions.assign(docids_positions.size(), {0, false});
//    for (const auto& word : words) {
//      for (const auto [docid, hitcount] : index.GetAccess().ref_to_value.Lookup(word)) {
//    	  if (docids_positions[docid].second) {
//    		  search_results[docids_positions[docid].first].second += hitcount;
//    	  } else {
//			  docids_positions[docid] = {search_results.size(), true};
//			  search_results.push_back({docid, hitcount});
//    	  }
//      }
//    }
//    partial_sort(
//      begin(search_results),
//	  end(search_results) - begin(search_results) > 5 ?
//			  begin(search_results) + 5 :
//			  end(search_results),
//      end(search_results),
//      [](pair<size_t, size_t> lhs, pair<size_t, size_t> rhs) {
//        int64_t lhs_docid = lhs.first;
//        auto lhs_hit_count = lhs.second;
//        int64_t rhs_docid = rhs.first;
//        auto rhs_hit_count = rhs.second;
//        return make_pair(lhs_hit_count, -lhs_docid) > make_pair(rhs_hit_count, -rhs_docid);
//      }
//    );
//    search_results_output << current_query << ':';
//    for (const auto& [docid, hitcount] : Head(search_results, 5)) {
//    	if (hitcount > 0) {
//		  search_results_output << " {"
//			<< "docid: " << docid << ", "
//			<< "hitcount: " << hitcount << '}';
//    	}
//    }
//    search_results_output << "\n";
//  }
}

void SearchServer::AddQueriesStringST(vector<string> queries,
		ostream &search_results_output) {
	vector<InvertedIndex::Entry> search_results;
	vector<pair<size_t, bool>> docids_positions(
			index.GetAccess().ref_to_value.GetDocsCount());
	for (string &current_query : queries) {
		const auto words = SplitIntoWords(current_query);
		search_results.clear();
		docids_positions.assign(docids_positions.size(), { 0, false });
		for (const auto &word : words) {
			for (const auto [docid, hitcount] : index.GetAccess().ref_to_value.Lookup(
					word)) {
				if (docids_positions[docid].second) {
					search_results[docids_positions[docid].first].hitcount +=
							hitcount;
				} else {
					docids_positions[docid] = { search_results.size(), true };
					search_results.push_back( { docid, hitcount });
				}
			}
		}
		partial_sort(begin(search_results),
				end(search_results) - begin(search_results) > 5 ?
						begin(search_results) + 5 : end(search_results),
				end(search_results),
				[](InvertedIndex::Entry lhs, InvertedIndex::Entry rhs) {
					int64_t lhs_docid = lhs.docid;
					auto lhs_hit_count = lhs.hitcount;
					int64_t rhs_docid = rhs.docid;
					auto rhs_hit_count = rhs.hitcount;
					return make_pair(lhs_hit_count, -lhs_docid)
							> make_pair(rhs_hit_count, -rhs_docid);
				}
		);
		search_results_output << current_query << ':';
		for (const auto& [docid, hitcount] : Head(search_results, 5)) {
			if (hitcount > 0) {
				search_results_output << " {" << "docid: " << docid << ", "
						<< "hitcount: " << hitcount << '}';
			}
		}
		search_results_output << "\n";
	}
}

void InvertedIndex::Add(string document) {
	docs.push_back(move(document));

	const size_t docid = docs.size() - 1;
	for (const auto &word : SplitIntoWords(docs.back())) {
		if (index[word].empty() || index[word].back().docid != docid) {
			index[word].push_back( { docid, 1 });
		} else {
			index[word].back().hitcount++;
		}
	}
}

const vector<InvertedIndex::Entry>& InvertedIndex::Lookup(
		const string &word) const {
	static const vector<InvertedIndex::Entry> empty;
	if (auto it = index.find(word); it != index.end()) {
		return it->second;
	} else {
		return empty;
	}
}

void InvertedIndex::Swap(InvertedIndex &new_index, size_t &docs_size) {
	docs_size = new_index.GetDocsCount();
	index.swap(new_index.index);
	docs.swap(new_index.docs);
}

void InvertedIndex::PrintAll() const {
	PrintIndex();
	PrintDocuments();
}

void InvertedIndex::PrintIndex() const {
	cout << "Index:\n";
	for (const auto& [word, hitcounts] : index) {
		cout << word << ": ";
		for (const auto& [docid, count] : hitcounts) {
			cout << "{" << docid << " - " << count << "} ";
		}
		cout << "\n";
	}
}

void InvertedIndex::PrintDocuments() const {
	cout << "Documents:\n";
	for (size_t docid = 0; docid < docs.size(); ++docid) {
		cout << docid << ": " << docs[docid] << "\n";
	}
}
