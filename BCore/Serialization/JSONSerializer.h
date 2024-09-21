#pragma once
#include "../String/StringView.h"
#include "../String/StringBuilder.h"
#include "../Containers/ArrayView.h"
#include "../Allocators/Allocator.h"
#include <ctype.h>

enum JSONNodeType
{
    Object,
    String,
    Array,
    Integer,
    Float
};

struct JSONNode
{
    JSONNodeType node_type;
    StringView name;
    StringView value;
    ArrayView<JSONNode> sub_nodes;
};

struct JSONSerializerState
{
    size_t current_index;
};

struct JSONSerializer
{
    static const inline char spaces[] = " \t\n\r";

    static void Log(JSONNode *in_node, StringBuilder *in_builder, size_t indentation)
    {
        for (size_t i = 0; i < indentation; ++i)
        {
            StringBuilder::Append(in_builder, "\t");
        }

        // Log type
        switch (in_node->node_type)
        {
        case JSONNodeType::Object:
        {
            StringBuilder::Append(in_builder, "<NodeType : Object>");
            break;
        }

        case JSONNodeType::Array:
        {
            StringBuilder::Append(in_builder, "<NodeType : Array>");
            break;
        }

        case JSONNodeType::Integer:
        {
            StringBuilder::Append(in_builder, "<NodeType : Integer>");
            break;
        }

        case JSONNodeType::String:
        {
            StringBuilder::Append(in_builder, "<NodeType : String>");
            break;
        }

        default:
            break;
        }

        StringBuilder::Append(in_builder, "\t");
        
        // Log name
        if (in_node->name.buffer != nullptr)
        {
            StringBuffer msg = StringUtils::Format(in_builder->alloc, "<Name : {}>\t", in_node->name);
            StringBuilder::Append(in_builder, msg.view);
        }

        // Log value
        if(in_node->node_type != JSONNodeType::Object && in_node->node_type != JSONNodeType::Array)
        {
            StringBuffer msg = StringUtils::Format(in_builder->alloc, "<Value : {}>\t", in_node->value);
            StringBuilder::Append(in_builder, msg.view);
        }

        StringBuilder::Append(in_builder, "\n");

        for (size_t i = 0; i < in_node->sub_nodes.size; ++i)
        {
            JSONNode *sub = &in_node->sub_nodes.data[i];

            Log(sub, in_builder, indentation + 1);
        }
    }

    static void Serialize(StringView json, JSONSerializerState *state, JSONNode *out_node, Allocator alloc)
    {
        assert(state->current_index <= json.length);

        ContinueWhileSpace(json, state);

        if (state->current_index == json.length)
        {
            return;
        }

        char curr_char = json.buffer[state->current_index];

        out_node->sub_nodes = {};
        out_node->value = {};

        if (curr_char == '{')
        {
            SerializeObject(json, state, out_node, alloc);
            return;
        }

        if (curr_char == '[')
        {
            SerializeArray(json, state, out_node, alloc);
            return;
        }

        if (curr_char == '"')
        {
            SerializeString(json, state, out_node, alloc);
            return;
        }

        if ('0' <= curr_char && curr_char <= '9')
        {
            SerializeInteger(json, state, out_node, alloc);
            return;
        }
    }

    static void SerializeString(StringView json, JSONSerializerState *state, JSONNode *inout_node, Allocator alloc)
    {
        bool stop = false;

        // skip the first '\"'
        state->current_index++;

        size_t start = state->current_index;

        while (!stop && state->current_index < json.length)
        {
            char curr_char = json.buffer[state->current_index];
            bool is_valid = curr_char != '"';
            stop |= !is_valid;

            state->current_index++;
        }

        inout_node->node_type = JSONNodeType::String;
        inout_node->value = json.buffer + start;
        // we subtract 1 from length to avoid including the last '\"' in the string value
        inout_node->value.length = state->current_index - start - 1;

        // skip the last '\"'
        state->current_index++;
    }

    void static SerializeInteger(StringView json, JSONSerializerState *state, JSONNode *inout_node, Allocator alloc)
    {
        bool stop = false;

        size_t start = state->current_index;

        while (!stop && state->current_index < json.length)
        {
            char curr_char = json.buffer[state->current_index];
            bool is_valid = '0' <= curr_char && curr_char <= '9';
            stop |= !is_valid;

            state->current_index++;
        }

        inout_node->node_type = JSONNodeType::Integer;
        inout_node->value = json.buffer + start;
        inout_node->value.length = state->current_index - start - 1;
    }

    void static SerializeObject(StringView json, JSONSerializerState *state, JSONNode *out_node, Allocator alloc)
    {
        out_node->node_type = JSONNodeType::Object;

        // skip first '{'
        state->current_index++;

        // skip spaces
        ContinueWhileSpace(json, state);

        DArray<JSONNode> sub_nodes = {};
        DArray<JSONNode>::Create(2, &sub_nodes, alloc);

        bool is_reading_object = true;

        while (is_reading_object)
        {
            size_t name_start = state->current_index;
            ContinueWhileAlphanum(json, state);

            // start sub node
            JSONNode sub_node = {};
            sub_node.name.buffer = json.buffer + name_start;
            sub_node.name.length = state->current_index - name_start;

            ContinueUntil(':', json, state);
            state->current_index++;

            ContinueWhileSpace(json, state);

            Serialize(json, state, &sub_node, alloc);

            DArray<JSONNode>::Add(&sub_nodes, sub_node);

            ContinueWhileSpace(json, state);

            char next_char = json.buffer[state->current_index];
            is_reading_object = next_char == ',';

            if (is_reading_object)
            {
                state->current_index++;
                ContinueWhileSpace(json, state);
            }
        }

        out_node->sub_nodes.data = sub_nodes.data;
        out_node->sub_nodes.size = sub_nodes.size;

        // skip the last ']'
        state->current_index++;
    }

    void static SerializeArray(StringView json, JSONSerializerState *state, JSONNode *out_node, Allocator alloc)
    {
        out_node->node_type = JSONNodeType::Array;

        // skip first '['
        state->current_index++;

        // skip spaces
        ContinueWhileSpace(json, state);

        DArray<JSONNode> sub_nodes = {};
        DArray<JSONNode>::Create(2, &sub_nodes, alloc);

        bool is_reading_object = true;

        while (is_reading_object)
        {
            ContinueWhileSpace(json, state);

            // start sub node
            JSONNode sub_node = {};
            Serialize(json, state, &sub_node, alloc);

            DArray<JSONNode>::Add(&sub_nodes, sub_node);

            ContinueWhileSpace(json, state);

            char next_char = json.buffer[state->current_index];
            is_reading_object = next_char == ',';

            if (is_reading_object)
            {
                state->current_index++;
            }
        }

        out_node->sub_nodes.data = sub_nodes.data;
        out_node->sub_nodes.size = sub_nodes.size;

        // skip the last ']'
        state->current_index++;
    }

    void static ContinueUntil(char character, StringView json, JSONSerializerState *state)
    {
        char curr_char = json.buffer[state->current_index];

        while (curr_char != character && state->current_index < json.length)
        {
            state->current_index++;
            curr_char = json.buffer[state->current_index];
        }
    }

    void static ContinueWhileAlphanum(StringView json, JSONSerializerState *state)
    {
        bool stop = false;
        const size_t count = sizeof(spaces) / sizeof(char);

        while (!stop && state->current_index < json.length)
        {
            char curr_char = json.buffer[state->current_index];
            bool is_valid = isalnum(curr_char);
            stop |= !is_valid;

            if (stop)
            {
                break;
            }

            state->current_index++;
        }
    }

    void static ContinueWhileSpace(StringView json, JSONSerializerState *state)
    {
        const size_t count = sizeof(spaces) / sizeof(char);

        while (state->current_index < json.length)
        {
            char curr_char = json.buffer[state->current_index];
            bool is_space = false;

            for (size_t i = 0; i < count; ++i)
            {
                char space = spaces[i];
                is_space |= curr_char == space;
            }

            if (!is_space)
            {
                break;
            }

            state->current_index++;
        }
    }

    void static ContinueWhile(char character, StringView json, JSONSerializerState *state)
    {
        char curr_char = json.buffer[state->current_index];

        while (curr_char == character && state->current_index < json.length)
        {
            state->current_index++;
        }
    }
};