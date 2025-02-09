#pragma once
#include "../String/StringView.h"
#include "../String/StringBuilder.h"
#include "../Containers/ArrayView.h"
#include "../Containers/Stack.h"
#include "../Allocators/Allocator.h"
#include <ctype.h>

enum class XMLNodeType
{
    Element,
    Text
};

struct AttributeValuePair
{
    StringView name;
    StringView value;
};

struct XMLNode
{
    XMLNodeType node_type;
    StringView name;
    StringView value;
    ArrayView<AttributeValuePair> attributes;
    ArrayView<XMLNode> sub_elements;
};

struct XMLSerializerState
{
    size_t current_index;
};

struct XMLSerializer
{
    static bool Validate(StringView xml, Allocator temp_alloc)
    {
        Stack<char> stack = {};
        Stack<char>::Create(&stack, 32, temp_alloc);
        bool is_in_string = false;

        for (size_t i = 0; i < xml.length; ++i)
        {
            char curr_char = xml.buffer[i];

            switch (curr_char)
            {
            case '{':
            {
                if (!is_in_string)
                {
                    Stack<char>::Push(&stack, '{');
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

                if (!Stack<char>::TryPop(&stack, &poped))
                {
                    return false;
                }

                if (poped != '{')
                {
                    return false;
                }

                break;
            }
            case '[':
            {
                if (!is_in_string)
                {
                    Stack<char>::Push(&stack, '[');
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

                if (!Stack<char>::TryPop(&stack, &poped))
                {
                    return false;
                }

                if (poped != '[')
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

    static void Print(XMLNode *in_node, StringBuilder *in_builder, size_t indentation)
    {
        for (size_t i = 0; i < indentation; ++i)
        {
            StringBuilder::Append(in_builder, "\t");
        }

        char temp_buffer[1024] = {0};

        Arena temp_arena = {};
        temp_arena.data = &temp_buffer;
        temp_arena.capacity = 1024;
        temp_arena.offset = 0;

        Allocator temp_alloc = ArenaAllocator::Create(&temp_arena);

        // Log element
        if (in_node->node_type == XMLNodeType::Element)
        {
            StringBuilder::Append(in_builder, "<Element : ");
            StringBuilder::Append(in_builder , in_node->name);
            for (size_t i = 0; i < in_node->attributes.size; ++i)
            {
                AttributeValuePair attrVal = in_node->attributes.data[i];

                StringBuffer atr = StringUtils::Format(in_builder->alloc, temp_alloc, "\t[Attribute : {} , Value : {}]", attrVal.name, attrVal.value);
                StringBuilder::Append(in_builder, atr.view);
            }
            StringBuilder::Append(in_builder, ">");
        }

        // Log text
        if (in_node->node_type == XMLNodeType::Text)
        {
            StringBuffer msg = StringUtils::Format(in_builder->alloc,temp_alloc, "<Text : {}>", in_node->value);
            StringBuilder::Append(in_builder, msg.view);
        }

        for (size_t i = 0; i < in_node->sub_elements.size; ++i)
        {
            XMLNode *sub = &in_node->sub_elements.data[i];

            StringBuilder::Append(in_builder, "\n");
            Print(sub, in_builder, indentation + 1);
        }

        // close
        if (in_node->node_type == XMLNodeType::Element)
        {
            StringBuilder::Append(in_builder, "\n");
            for (size_t i = 0; i < indentation; ++i)
            {
                StringBuilder::Append(in_builder, "\t");
            }

            StringBuffer name = StringUtils::Format(in_builder->alloc, temp_alloc,"</Element : {}>", in_node->name);
            StringBuilder::Append(in_builder, name.view);
        }
    }

    static void Serialize(StringView xml, XMLSerializerState *state, XMLNode *out_node, Allocator alloc)
    {
        assert(state->current_index <= xml.length);

        ContinueWhileSpace(xml, state);

        if (state->current_index == xml.length)
        {
            return;
        }

        char curr_char = xml.buffer[state->current_index];

        out_node->sub_elements = {};
        out_node->value = {};

        if (curr_char == '<')
        {
            SerializeObject(xml, state, out_node, alloc);
            return;
        }

        else
        {
            SerializeText(xml, state, out_node, alloc);
            return;
        }
    }

    static void SerializeText(StringView xml, XMLSerializerState *state, XMLNode *inout_node, Allocator alloc)
    {
        bool stop = false;

        size_t start = state->current_index;

        while (!stop && state->current_index < xml.length)
        {
            char curr_char = xml.buffer[state->current_index];

            bool is_valid =
                isalnum(curr_char) ||
                curr_char == ' ' ||
                curr_char == '\t' ||
                curr_char == '\r' ||
                curr_char == '\n';

            stop |= !is_valid;

            state->current_index++;
        }

        state->current_index--;

        inout_node->node_type = XMLNodeType::Text;
        inout_node->value = xml.buffer + start;
        inout_node->value.length = state->current_index - start;
    }

    void static SerializeObject(StringView xml, XMLSerializerState *state, XMLNode *out_node, Allocator alloc)
    {
        out_node->node_type = XMLNodeType::Element;

        // skip first '<'
        state->current_index++;

        // skip spaces
        ContinueWhileSpace(xml, state);

        // get element name
        {
            size_t name_start = state->current_index;
            ContinueWhileAlphanum(xml, state);

            out_node->name.buffer = xml.buffer + name_start;
            out_node->name.length = state->current_index - name_start;
        }

        // read attributes
        {
            DArray<AttributeValuePair> attributes = {};
            DArray<AttributeValuePair>::Create(2, &attributes, alloc);

            bool is_reading_attributes = true;

            while (is_reading_attributes)
            {
                // skip spaces
                ContinueWhileSpace(xml, state);

                char curr_char = xml.buffer[state->current_index];
                char next_char = xml.buffer[state->current_index + 1];

                // check for early exit
                if (curr_char == '>' || (curr_char == '/' && next_char == '>'))
                {
                    is_reading_attributes = false;
                    break;
                }

                // attribute name
                AttributeValuePair attr = {};
                {
                    size_t attr_start = state->current_index;
                    ContinueWhileAlphanum(xml, state);

                    attr.name.buffer = xml.buffer + attr_start;
                    attr.name.length = state->current_index - attr_start;
                }

                // skip space
                ContinueWhileSpace(xml, state);

                // add attribute without value
                if (xml.buffer[state->current_index] == '=')
                {
                    // skip '='
                    state->current_index++;

                    // skip space
                    ContinueWhileSpace(xml, state);

                    if (xml.buffer[state->current_index] == '"')
                    {
                        state->current_index++;
                    }

                    // get attr value
                    {
                        size_t attr_start = state->current_index;
                        ContinueWhileAlphanum(xml, state);

                        attr.value.buffer = xml.buffer + attr_start;
                        attr.value.length = state->current_index - attr_start;
                    }

                    if (xml.buffer[state->current_index] == '"')
                    {
                        state->current_index++;
                    }
                }

                DArray<AttributeValuePair>::Add(&attributes, attr);
            }

            out_node->attributes.data = attributes.data;
            out_node->attributes.size = attributes.size;
        }

        // check early end without sub elements
        {
            char curr_char = xml.buffer[state->current_index];
            char next_char = xml.buffer[state->current_index + 1];

            if (curr_char == '/' && next_char == '>')
            {
                state->current_index += 2;
                return;
            }
        }

        // skip space
        ContinueWhileSpace(xml, state);

        // skip '>'
        state->current_index++;

        // skip space
        ContinueWhileSpace(xml, state);

        // read sub elements
        {
            DArray<XMLNode> sub_elems = {};
            DArray<XMLNode>::Create(2, &sub_elems, alloc);

            bool is_reading_sub_elem = true;

            while (is_reading_sub_elem)
            {
                char curr_char = xml.buffer[state->current_index];
                char next_char = xml.buffer[state->current_index + 1];

                // if closing elem
                if (curr_char == '<' && next_char == '/')
                {
                    state->current_index += 2;

                    ContinueWhileSpace(xml, state);

                    // skip name
                    ContinueWhileAlphanum(xml, state);

                    ContinueWhileSpace(xml, state);

                    // skip '>'
                    state->current_index++;
                    is_reading_sub_elem = false;
                    break;
                }

                XMLNode sub_elem = {};
                Serialize(xml, state, &sub_elem, alloc);

                DArray<XMLNode>::Add(&sub_elems, sub_elem);
            }

            out_node->sub_elements.data = sub_elems.data;
            out_node->sub_elements.size = sub_elems.size;
        }
    }

    void static ContinueUntil(char character, StringView xml, XMLSerializerState *state)
    {
        char curr_char = xml.buffer[state->current_index];

        while (curr_char != character && state->current_index < xml.length)
        {
            state->current_index++;
            curr_char = xml.buffer[state->current_index];
        }
    }

    void static ContinueWhileAlphanum(StringView xml, XMLSerializerState *state)
    {
        bool stop = false;

        while (!stop && state->current_index < xml.length)
        {
            char curr_char = xml.buffer[state->current_index];
            bool is_valid = isalnum(curr_char);
            stop |= !is_valid;

            if (stop)
            {
                break;
            }

            state->current_index++;
        }
    }

    void static ContinueWhileSpace(StringView xml, XMLSerializerState *state)
    {
        while (state->current_index < xml.length)
        {
            char curr_char = xml.buffer[state->current_index];
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

    void static ContinueWhile(char character, StringView xml, XMLSerializerState *state)
    {
        char curr_char = xml.buffer[state->current_index];

        while (curr_char == character && state->current_index < xml.length)
        {
            state->current_index++;
        }
    }
};