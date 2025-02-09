#pragma once
#include "../String/StringView.h"
#include "../String/StringBuilder.h"
#include "../Containers/ArrayView.h"
#include "../Containers/Stack.h"
#include "../Allocators/Allocator.h"
#include <ctype.h>

enum class JSONNodeType
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
    static bool Validate(StringView json , Allocator temp_alloc)
    {
        Stack<char> stack = {};
        Stack<char>::Create(&stack , 32 , temp_alloc);
        bool is_in_string = false;

        for (size_t i = 0; i < json.length; ++i)
        {
            char curr_char = json.buffer[i];

            switch (curr_char)
            {
            case '{':
            {
                if (!is_in_string)
                {
                    Stack<char>::Push(&stack , '{');
                }
                break;
            }
            case '}':
            {
                if (is_in_string)
                {
                    break;
                }

                char poped = {};

                if(!Stack<char>::TryPop(&stack , &poped))
                {
                    return false;
                }

                if(poped != '{')
                {
                    return false;
                }

                break;
            }
            case '[':
            {
                if (!is_in_string)
                {
                    Stack<char>::Push(&stack , '[');
                }
                break;
            }
            case ']':
            {
                if (is_in_string)
                {
                    break;
                }

                char poped = {};
                
                if(!Stack<char>::TryPop(&stack , &poped))
                {
                    return false;
                }

                if(poped != '[')
                {
                    return false;
                }

                break;
            }
            case '"':
            {
                is_in_string = !is_in_string;
                break;
            }
            }
        }
        bool is_valid = !is_in_string && stack.size == 0;

        return is_valid;
    }

    static void Print(JSONNode *in_node, StringBuilder *in_builder, size_t indentation)
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

        case JSONNodeType::Float:
        {
            StringBuilder::Append(in_builder, "<NodeType : Float>");
            break;
        }

        default:
            break;
        }

        StringBuilder::Append(in_builder, "\t");
        
        char temp_buffer[1024] = {0};

        Arena temp_arena = {};
        temp_arena.data = &temp_buffer;
        temp_arena.capacity = 1024;
        temp_arena.offset = 0;

        Allocator temp_alloc = ArenaAllocator::Create(&temp_arena);
        
        // Log name
        if (in_node->name.buffer != nullptr)
        {
            StringBuffer msg = StringUtils::Format(in_builder->alloc, temp_alloc, "<Name : {}>\t", in_node->name);
            StringBuilder::Append(in_builder, msg.view);
        }

        // Log value
        if (in_node->node_type != JSONNodeType::Object && in_node->node_type != JSONNodeType::Array)
        {
            StringBuffer msg = StringUtils::Format(in_builder->alloc, temp_alloc, "<Value : {}>\t", in_node->value);
            StringBuilder::Append(in_builder, msg.view);
        }

        StringBuilder::Append(in_builder, "\n");

        for (size_t i = 0; i < in_node->sub_nodes.size; ++i)
        {
            JSONNode *sub = &in_node->sub_nodes.data[i];

            Print(sub, in_builder, indentation + 1);
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

        bool is_num = ('0' <= curr_char && curr_char <= '9') || (curr_char == '+') || (curr_char == '-');

        if (is_num)
        {
            SerializeNumber(json, state, out_node, alloc);
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
    }

    void static SerializeNumber(StringView json, JSONSerializerState *state, JSONNode *inout_node, Allocator alloc)
    {
        bool is_float = false;
        size_t start = state->current_index;

        int32_t sign_mul = 1;
        char curr_char = json.buffer[state->current_index];

        if (curr_char == '-')
        {
            sign_mul = -1;
            state->current_index++;
        }

        if (curr_char == '+')
        {
            state->current_index++;
        }

        bool done = false;

        do
        {
            curr_char = json.buffer[state->current_index];
            bool is_curr_num = isdigit(curr_char);
            bool is_frac = curr_char == '.';

            is_float |= is_frac;
            done = !is_curr_num && !is_frac;
            state->current_index++;
        } while (!done && state->current_index < json.length);

        JSONNodeType node_type = JSONNodeType::Integer;

        if (is_float)
        {
            node_type = JSONNodeType::Float;
        }

        state->current_index--;

        inout_node->node_type = node_type;
        inout_node->value = json.buffer + start;
        inout_node->value.length = state->current_index - start;
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
        while (state->current_index < json.length)
        {
            char curr_char = json.buffer[state->current_index];
            bool is_space =
                curr_char == ' ' ||
                curr_char == '\t' ||
                curr_char == '\r' ||
                curr_char == '\n';

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