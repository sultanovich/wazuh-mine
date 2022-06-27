/* Copyright (C) 2015-2022, Wazuh Inc.
 * All rights reserved.
 *
 */

#include <gtest/gtest.h>

#include <vector>

#include <baseTypes.hpp>

#include "opBuilderHelperFilter.hpp"
#include "testUtils.hpp"

using namespace base;
using namespace builder::internals::builders;

using FakeTrFn = std::function<void(std::string)>;
static FakeTrFn tr = [](std::string msg) {
};

// Build ok
TEST(opBuilderHelperStringStarts, Build)
{
    Document doc {R"({
        "check":
            {"sourceField": "+s_starts/test_value"}
    })"};
    ASSERT_NO_THROW(opBuilderHelperStringStarts(doc.get("/check"), tr));
}

// Build incorrect number of arguments
TEST(opBuilderHelperStringStarts, BuildManyParametersError)
{
    Document doc {R"({
        "check":
            {"sourceField": "+s_starts/test_value/test_value2"}
    })"};
    ASSERT_THROW(opBuilderHelperStringStarts(doc.get("/check"), tr), std::runtime_error);
}

// Test ok: static values
TEST(opBuilderHelperStringStarts, RawString)
{
    Document doc {R"({
        "check":
            {"sourceField": "+s_starts/test_value"}
    })"};

    Observable input = observable<>::create<Event>([=](auto s) {
        s.on_next(createSharedEvent(R"(
                {"sourceField":"sample_value"}
            )")); // Shall not pass
        s.on_next(createSharedEvent(R"(
                {"sourceField":"sample_value_test_value"}
            )")); // Shall not pass
        s.on_next(createSharedEvent(R"(
                {"sourceField":"test_valu"}
            )")); // Shall pass
        s.on_next(createSharedEvent(R"(
                {"sourceField":"test_value_extended"}
            )")); // Shall pass
        s.on_next(createSharedEvent(R"(
                {"otherfield":"value"}
            )")); // Shall not pass
        s.on_next(createSharedEvent(R"(
                {"otherfield":"test_value"}
            )")); // Shall not pass
        s.on_next(createSharedEvent(R"(
                {"sourceField":"test_value_extended"}
            )")); // Shall pass
        s.on_completed();
    });

    Lifter lift = [=](Observable input) {
        return input.filter(opBuilderHelperStringStarts(doc.get("/check"), tr));
    };
    Observable output = lift(input);
    vector<Event> expected;
    output.subscribe([&](Event e) { expected.push_back(e); });
    ASSERT_EQ(expected.size(), 2);
}

// Test ok: static values
TEST(opBuilderHelperStringStarts, nonString)
{
    Document doc {R"({
        "check":
            {"sourceField": "+s_starts/test_value"}
    })"};

    Observable input = observable<>::create<Event>([=](auto s) {
        s.on_next(createSharedEvent(R"(
                {"sourceField": null}
            )")); // Shall pass
        s.on_next(createSharedEvent(R"(
                {"sourceField": true}
            )")); // Shall pass
        s.on_next(createSharedEvent(R"(
                {"sourceField": 10}
            )")); // Shall pass
        s.on_next(createSharedEvent(R"(
                {"sourceField": ["hi", "bye"]}
            )")); // Shall pass
        s.on_next(createSharedEvent(R"(
                {"sourceField": { "a": "b" }}
            )")); // Shall pass
        s.on_completed();
    });

    Lifter lift = [=](Observable input) {
        return input.filter(opBuilderHelperStringStarts(doc.get("/check"), tr));
    };
    Observable output = lift(input);
    vector<Event> expected;
    output.subscribe([&](Event e) { expected.push_back(e); });
    ASSERT_EQ(expected.size(), 2);
}

// Test ok: static values
TEST(opBuilderHelperStringStarts, TruncatedRawString)
{
    Document doc {R"({
        "check":
            {"sourceField": "+s_starts/test_value"}
    })"};

    Observable input = observable<>::create<Event>([=](auto s) {
        s.on_next(createSharedEvent(R"(
                {"sourceField":"sample_value"}
            )")); // Shall not pass
        s.on_next(createSharedEvent(R"(
                {"sourceField":"test_value_extended"}
            )")); // Shall pass
        s.on_next(createSharedEvent(R"(
                {"otherfield":"value"}
            )")); // Shall not pass
        s.on_next(createSharedEvent(R"(
                {"otherfield":"test_value"}
            )")); // Shall not pass
        s.on_next(createSharedEvent(R"(
                {"sourceField":"test_value_extra_extended"}
            )")); // Shall pass
        s.on_next(createSharedEvent(R"(
                {"sourceField":"test_value"}
            )")); // Shall pass
        s.on_completed();
    });

    Lifter lift = [=](Observable input) {
        return input.filter(opBuilderHelperStringStarts(doc.get("/check"), tr));
    };
    Observable output = lift(input);
    vector<Event> expected;
    output.subscribe([&](Event e) { expected.push_back(e); });
    ASSERT_EQ(expected.size(), 3);
}

// Test ok: dynamic values (string)
TEST(opBuilderHelperStringStarts, ReferencedString)
{
    Document doc {R"({
        "check":
            {"sourceField": "+s_starts/$ref_key"}
    })"};

    Observable input = observable<>::create<Event>([=](auto s) {
        s.on_next(createSharedEvent(R"(
                {
                    "sourceField":"sample_value",
                    "ref_key":"test_value"
                }
            )")); // Shall not pass
        s.on_next(createSharedEvent(R"(
                {
                    "sourceField":"test_value",
                    "ref_key":"test_value"
                }
            )")); // Shall pass
        s.on_next(createSharedEvent(R"(
                {
                    "otherfield":"value",
                    "ref_key":"test_value"
                }
            )")); // Shall not pass
        s.on_next(createSharedEvent(R"(
                {
                    "otherfield":"test_value",
                    "ref_key":"test_value"
                }
            )")); // Shall not pass
        s.on_next(createSharedEvent(R"(
                {
                    "sourceField":"test_value_extended",
                    "ref_key":"test_value"
                }
            )")); // Shall pass
        s.on_next(createSharedEvent(R"(
                {
                    "sourceField":"test_value_extra_extended",
                    "ref_key":"test_value"
                }
            )")); // Shall pass
        s.on_completed();
    });

    Lifter lift = [=](Observable input) {
        return input.filter(opBuilderHelperStringStarts(doc.get("/check"), tr));
    };
    Observable output = lift(input);
    vector<Event> expected;
    output.subscribe([&](Event e) { expected.push_back(e); });
    ASSERT_EQ(expected.size(), 3);
}

// Test ok: multilevel referenced values (string)
TEST(opBuilderHelperStringStarts, NestedReferencedStrings)
{
    Document doc {R"({
        "check":
            {"rootKey1.sourceField": "+s_starts/$rootKey2.ref_key"}
    })"};

    Observable input = observable<>::create<Event>([=](auto s) {
        s.on_next(createSharedEvent(R"(
                {
                    "rootKey2": {
                        "sourceField": "test_value",
                        "ref_key": "test_value"
                    },
                    "rootKey1": {
                        "sourceField": "sample_value",
                        "ref_key": "sample_value"
                    }
                }
            )")); // Shall not pass
        s.on_next(createSharedEvent(R"(
                {
                    "rootKey2": {
                        "sourceField": "test_value",
                        "ref_key": "sample_value"
                    },
                    "rootKey1": {
                        "sourceField": "sample_value",
                        "ref_key": "test_value"
                    }
                }
            )")); // Shall pass
        s.on_next(createSharedEvent(R"(
                {
                    "rootKey2": {
                        "sourceField": "test_value_extended",
                        "ref_key": "sample_value"
                    },
                    "rootKey1": {
                        "sourceField": "sample_value",
                        "ref_key": "test_value"
                    }
                }
            )")); // Shall pass
        s.on_next(createSharedEvent(R"(
                {
                    "rootKey2": {
                        "sourceField": "test_value_extended",
                        "ref_key": "sample_value"
                    },
                    "rootKey1": {
                        "sourceField": "sample_value",
                        "ref_key": "test_value_extra_extended"
                    }
                }
            )")); // Shall pass
        s.on_completed();
    });

    Lifter lift = [=](Observable input) {
        return input.filter(opBuilderHelperStringStarts(doc.get("/check"), tr));
    };
    Observable output = lift(input);
    vector<Event> expected;
    output.subscribe([&](Event e) { expected.push_back(e); });
    ASSERT_EQ(expected.size(), 3);
}
