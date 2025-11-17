#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "Trace.h"

#ifndef MATCHER_P
#error "matcher_p is not defined here"
#endif

namespace gtestverilog {

    namespace matches_trace {
        bool compare(const Trace& a, const Trace& b, ::testing::MatchResultListener& listener);
    }

    MATCHER_P(MatchesTrace, trace, "Matches Trace") {
        if (!matches_trace::compare(arg, trace, *result_listener)) {
            return false;
        }

        return true;
    }
}