#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void exeception_for_invalid_pos(Position pos) {
    if(!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }
}

void Sheet::SetCell(Position pos, std::string text) {
    exeception_for_invalid_pos(pos);
    //Если позиция больше текущего размера , увеличиваю развмер вдвое, так же увеличиваю размер строк в случае добавления новых
    if(static_cast<size_t>(pos.row) >= cells_.size()) {
        cells_.resize((pos.row == 0 ? 1:pos.row)*2);
        for(size_t i = 0; i < cells_.size(); ++i) {
            cells_[i].resize(cells_[0].size());
        }
    }
    //Если колонка юолше чем текущий размер строки , то увеличиваю размер строк вдвое
    if(static_cast<size_t>(pos.col) >= cells_[0].size()) {
        for(size_t i = 0; i < cells_.size(); ++i) {
            cells_[i].resize((pos.col == 0 ? 1:pos.col)*2);
        }
    }
    if(pos.row + 1 > max_print_size_row) {
        max_print_size_row = pos.row + 1;
    }
    if(pos.col + 1 > max_print_size_col) {
        max_print_size_col = pos.col + 1;
    }

    if(!cells_[pos.row][pos.col]) {
       cells_[pos.row][pos.col] = std::make_unique<Cell>(*this);
    }

    //cells_[pos.row][pos.col]->Set(text);
    dynamic_cast<Cell*>(cells_.at(pos.row).at(pos.col).get())->Set(std::move(text));
}

const CellInterface* Sheet::GetCell(Position pos) const {

    exeception_for_invalid_pos(pos);

    if(cells_.empty()) {
        return nullptr;
    }

    if(static_cast<size_t>(pos.row) < cells_.size() && static_cast<size_t>(pos.col) < cells_[0].size()) {
        if(cells_[pos.row][pos.col] != nullptr) {
            return cells_[pos.row][pos.col].get();
        } else {

        }
    }
    return nullptr;
}
CellInterface* Sheet::GetCell(Position pos) {

    exeception_for_invalid_pos(pos);

    if(cells_.empty()) {
        return nullptr;
    }
    if(static_cast<size_t>(pos.row) < cells_.size() && static_cast<size_t>(pos.col) < cells_[0].size()) {
        if(cells_[pos.row][pos.col] != nullptr) {
            return cells_[pos.row][pos.col].get();
        }
    }
    return nullptr;
}


void Sheet::ClearCell(Position pos) {
    exeception_for_invalid_pos(pos);

    if(static_cast<size_t>(pos.row) < cells_.size() && static_cast<size_t>(pos.col) < cells_[0].size() && cells_[pos.row][pos.col] != nullptr ) {
        cells_[pos.row][pos.col]->Clear();
        cells_[pos.row][pos.col].reset(nullptr);
    }
    //Когда удаляется последний
    if(max_print_size_row == 1 && max_print_size_col == 1 && pos == Position{0,0}) {
        max_print_size_row = 0;
        max_print_size_col = 0;
        return;
    }

    //При удалении элемента который обладал крайней нижней позийией(строка) нужно найти ближайший столбец хранящий не нулптр
    if(pos.row + 1 == max_print_size_row) {
        bool flag = false;
        for(int i = max_print_size_row; i > 0; --i) {
            if(flag) break;
            for(int j = 0; j < max_print_size_col; ++j) {
                if(cells_[i - 1][j]) {
                    max_print_size_row = i;
                    flag = true;
                }
            }
        }
    }

    //При удалении элемента который обладал крайней правой позийией(столбец) нужно найти ближайший столбец хранящий не нулптр
    if(pos.col + 1 == max_print_size_col) {
        for(int i = max_print_size_col; i > 0; --i) {
            for(int j = 0; j < max_print_size_row; j++) {
                if(cells_[j][i-1]) {
                    max_print_size_col = i;
                    return;
                }
            }
        }
    }
}

Size Sheet::GetPrintableSize() const {
    return Size{max_print_size_row, max_print_size_col};
}

void Sheet::PrintValues(std::ostream& output) const {
    for(int i = 0; i < max_print_size_row ; ++i) {
        for(int j = 0; j < max_print_size_col ; ++j) {
            if(cells_[i][j]) {
                CellInterface::Value v = cells_[i][j]->GetValue();
                if(std::holds_alternative<double>(v)) {
                    output << std::get<double>(v);
                }
                if(std::holds_alternative<FormulaError>(v)) {
                    output << std::get<FormulaError>(v);
                }
                if(std::holds_alternative<std::string>(v)) {
                    output << std::get<std::string>(v) ;
                }

            }
            if(j + 1 != max_print_size_col) {
                output << "\t";
            }
        }
        output << "\n";
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    for(int i = 0; i < max_print_size_row ; ++i) {
        for(int j = 0; j < max_print_size_col ; ++j) {
            if(cells_[i][j]) {
                output << cells_[i][j]->GetText();
            }
            if(j + 1 != max_print_size_col) {
                output << "\t";
            }
        }
        output << "\n";
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
