#pragma once
#include "EntryPoint.h"
#include "TextUI.h"

struct GameText
{
    static void Build(EntryPoint *inst)
    {
        bool is_empty = inst->text.instance_matricies.size == 0 && inst->text.instance_matricies.start == 0;
        if (!is_empty)
        {
            TextUI::Destroy(&inst->text);
        }

        inst->text = TextUI::Create("Hello there, gottem !!!");
    };
};