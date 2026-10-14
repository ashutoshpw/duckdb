#pragma once

#include "duckdb/parser/peg/matcher.hpp"

namespace duckdb {

//! Zero-width matcher that fails when the upcoming tokens start an AT (VERSION|TIMESTAMP => ...) time travel clause.
class NotAtClauseMatcher : public AtomicMatcher {
public:
	static constexpr MatcherType TYPE = MatcherType::CUSTOM;

public:
	NotAtClauseMatcher() : AtomicMatcher(TYPE) {
	}

	MatcherResult MatchAtomic(MatchState &state) const override {
		auto &iterator = state.token_iterator;
		auto position = iterator.Position();
		if (position + 3 < iterator.Size() && IsText(iterator.GetToken(position), "AT") &&
		    IsText(iterator.GetToken(position + 1), "(") &&
		    (IsText(iterator.GetToken(position + 2), "VERSION") ||
		     IsText(iterator.GetToken(position + 2), "TIMESTAMP")) &&
		    IsText(iterator.GetToken(position + 3), "=>")) {
			return MatcherResult::Failure();
		}
		return state.AllocateParseResult<TokenParseResult>(string(), optional_idx(), optional_idx());
	}

	SuggestionType AddSuggestionInternal(MatchState &state) const override {
		return SuggestionType::OPTIONAL;
	}

	string ToString() const override {
		return "NotAtClause";
	}

private:
	static bool IsText(const MatcherToken &token, const char *text) {
		return StringUtil::CIEquals(token.text, text);
	}
};

} // namespace duckdb
