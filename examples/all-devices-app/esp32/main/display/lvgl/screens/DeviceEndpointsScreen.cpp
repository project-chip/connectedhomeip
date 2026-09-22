/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "DeviceEndpointsScreen.h"
#include "DeviceScreenRegistry.h"
#include "DeviceTypeSelection.h"
#include "NavigationStack.h"

#include <cstdio>
#include <string>
#include <vector>

namespace {

// Deep enough for aggregator -> bridged node -> device -> composed part, and a guard
// against a malformed parent chain.
constexpr uint8_t kMaxEndpointDepth = 6;

// Indent per tree level, in spaces prefixed to the cell text.
constexpr uint8_t kIndentSpaces = 3;

// The endpoint lives in its own column so that a long title is truncated rather than pushing
// it out of the row. That column is also the hit target: taps elsewhere in the row do
// nothing, so scrolling the list cannot open a device by accident.
constexpr uint32_t kEndpointColumn     = 1;
constexpr int32_t kEndpointColumnWidth = 70;

// Marks rows whose device type has no screen.
constexpr lv_table_cell_ctrl_t kNoScreenCellCtrl = LV_TABLE_CELL_CTRL_CUSTOM_1;

// Keeps every cell on a single line; without it the table grows the row to fit wrapped text.
constexpr lv_table_cell_ctrl_t kCropCellCtrl = LV_TABLE_CELL_CTRL_TEXT_CROP;

// Scroll offset of the list, kept across navigation so that returning from a device screen
// lands where the list was left.
int32_t sSavedScrollY = 0;

void OnTableDeleted(lv_event_t * event)
{
    sSavedScrollY = lv_obj_get_scroll_y(lv_event_get_target_obj(event));
}

// Sent by the table on release, and only when the press did not turn into a scroll.
void OnCellSelected(lv_event_t * event)
{
    lv_obj_t * table = lv_event_get_target_obj(event);

    uint32_t row = LV_TABLE_CELL_NONE;
    uint32_t col = LV_TABLE_CELL_NONE;
    lv_table_get_selected_cell(table, &row, &col);

    if (col != kEndpointColumn)
    {
        return;
    }

    // Copied out rather than pushed from inside the visitor: rendering the target screen can
    // read the registry again (AggregatorScreen does), which would re-enter the lock.
    std::string title;
    chip::app::DeviceScreenEntry::RenderFn renderFn;

    chip::app::DeviceScreenRegistry::Instance().WithEntries([&](const auto & entries) {
        if (row < entries.size())
        {
            title    = entries[row].title;
            renderFn = entries[row].renderFn;
        }
    });

    if (renderFn)
    {
        NavigationStack::Push(title, renderFn);
    }
}

// Rows without a screen are drawn in red. The table has no per-cell styles, so the colour is
// applied to the draw task of the marked cells.
void OnRowDrawTask(lv_event_t * event)
{
    lv_obj_t * table         = lv_event_get_target_obj(event);
    lv_draw_task_t * task    = lv_event_get_draw_task(event);
    lv_draw_dsc_base_t * dsc = static_cast<lv_draw_dsc_base_t *>(lv_draw_task_get_draw_dsc(task));

    if (dsc->part != LV_PART_ITEMS || lv_draw_task_get_type(task) != LV_DRAW_TASK_TYPE_LABEL)
    {
        return;
    }

    if (lv_table_has_cell_ctrl(table, dsc->id1, 0, kNoScreenCellCtrl))
    {
        static_cast<lv_draw_label_dsc_t *>(lv_draw_task_get_draw_dsc(task))->color = lv_palette_main(LV_PALETTE_RED);
    }
}

const chip::app::DeviceScreenEntry * FindEntry(const std::vector<chip::app::DeviceScreenEntry> & entries, chip::EndpointId id)
{
    for (const auto & entry : entries)
    {
        if (entry.endpointId == id)
        {
            return &entry;
        }
    }
    return nullptr;
}

// Distance to the root of the endpoint tree, used to indent the list. Ancestors without an
// entry of their own end the walk, so a device whose parent is not listed reads as a root.
uint8_t EntryDepth(const std::vector<chip::app::DeviceScreenEntry> & entries, const chip::app::DeviceScreenEntry & entry)
{
    uint8_t depth = 0;
    for (const auto * current = &entry; depth < kMaxEndpointDepth; ++depth)
    {
        if (current->parentEndpointId == chip::kInvalidEndpointId)
        {
            break;
        }
        current = FindEntry(entries, current->parentEndpointId);
        if (current == nullptr)
        {
            break;
        }
    }
    return depth;
}

void ShowNoEntriesPlaceholder(lv_obj_t * parent)
{
    const std::string activeDev = GetActiveDeviceType();
    bool isAllBridged           = (activeDev == "*" || activeDev == "aggregator");

    lv_obj_t * card = lv_obj_create(parent);
    lv_obj_set_width(card, LV_PCT(100));
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(card, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_row(card, 6, LV_PART_MAIN);

    lv_obj_t * header = lv_label_create(card);
    lv_label_set_text_static(header, "Device Screens");
    lv_obj_set_style_text_color(header, lv_palette_lighten(LV_PALETTE_BLUE, 2), LV_PART_MAIN);

    lv_obj_t * configLabel = lv_label_create(card);
    std::string configText = "Configured: " + (isAllBridged ? std::string("All Bridged (*)") : activeDev);
    lv_label_set_text(configLabel, configText.c_str());

    lv_obj_t * todoNote = lv_label_create(parent);
    lv_obj_set_width(todoNote, LV_PCT(100));
    lv_label_set_text_static(todoNote,
                             "No interactive device screens are registered for the current configuration.\n\n"
                             "Screens register dynamically during device construction via DeviceFactory hooks.");
    lv_obj_set_style_text_color(todoNote, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);
}

void BuildEntryTable(lv_obj_t * parent, const std::vector<chip::app::DeviceScreenEntry> & entries)
{
    lv_obj_update_layout(parent);
    const int32_t listWidth = lv_obj_get_content_width(parent);

    lv_obj_t * table = lv_table_create(parent);
    lv_obj_set_size(table, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_pad_ver(table, 10, LV_PART_ITEMS);
    lv_obj_set_style_pad_hor(table, 8, LV_PART_ITEMS);

    lv_table_set_column_count(table, 2);
    lv_table_set_row_count(table, entries.size());
    lv_table_set_column_width(table, 0, listWidth - kEndpointColumnWidth);
    lv_table_set_column_width(table, kEndpointColumn, kEndpointColumnWidth);

    for (size_t i = 0; i < entries.size(); ++i)
    {
        const auto & entry  = entries[i];
        const uint8_t depth = EntryDepth(entries, entry);

        std::string title(static_cast<size_t>(depth) * kIndentSpaces, ' ');
        title += entry.title;

        lv_table_set_cell_value(table, i, 0, title.c_str());
        lv_table_set_cell_value_fmt(table, i, kEndpointColumn, "EP %u", static_cast<unsigned int>(entry.endpointId));

        lv_table_set_cell_ctrl(table, i, 0, kCropCellCtrl);
        lv_table_set_cell_ctrl(table, i, kEndpointColumn, kCropCellCtrl);

        if (!entry.renderFn)
        {
            lv_table_set_cell_ctrl(table, i, 0, kNoScreenCellCtrl);
        }
    }

    lv_obj_add_event_cb(table, OnCellSelected, LV_EVENT_VALUE_CHANGED, nullptr);
    lv_obj_add_event_cb(table, OnRowDrawTask, LV_EVENT_DRAW_TASK_ADDED, nullptr);
    lv_obj_add_event_cb(table, OnTableDeleted, LV_EVENT_DELETE, nullptr);
    lv_obj_set_send_draw_task_events(table, true);

    // Return to where the list was left. The layout pass gives the table its scroll range;
    // LVGL clamps the offset if the list got shorter.
    lv_obj_update_layout(parent);
    lv_obj_scroll_to_y(table, sSavedScrollY, LV_ANIM_OFF);
}

} // namespace

void ShowDeviceEndpoints(lv_obj_t * parent)
{
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(parent, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_row(parent, 8, LV_PART_MAIN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // The whole build runs inside the visitor: nothing below reads the registry again, so the
    // lock is held once and no entry reference outlives it.
    chip::app::DeviceScreenRegistry::Instance().WithEntries([parent](const auto & entries) {
        if (entries.empty())
        {
            ShowNoEntriesPlaceholder(parent);
            return;
        }
        BuildEntryTable(parent, entries);
    });
}
