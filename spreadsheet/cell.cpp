#include "cell.h"
#include "sheet.h"
#include <cassert>
#include <iostream>
#include <string>
#include <optional>


CellInterface::Value EmptyImpl::GetValue() const {
    return CellInterface::Value();
}

std::string EmptyImpl::GetText() const {
    return "";
}


TextImpl::TextImpl(std::string text):text_(text) {
}

CellInterface::Value TextImpl::GetValue() const {
    if(text_[0] == '\'' ) {
        return CellInterface::Value{text_.substr(1)};
    }

    return CellInterface::Value{text_};
}
std::string TextImpl::GetText() const {
    return text_;
}



FormulaImpl::FormulaImpl(std::string formula, SheetInterface& sheet) : sheet_(sheet), formula_(ParseFormula(std::move(formula))){
}

CellInterface::Value FormulaImpl::GetValue() const{
    FormulaInterface::Value value = formula_->Evaluate(sheet_);
    if (std::holds_alternative<double>(value)) {
        return std::get<double>(value);
    } else {
        return std::get<FormulaError>(value);
    }
}

std::string FormulaImpl::GetText() const {
    return "=" + formula_->GetExpression();
}
const FormulaInterface* FormulaImpl::GetFormulaInterface() const {
    return formula_.get();
}

// Реализуйте следующие методы
Cell::Cell(Sheet& sheet) : sheet_(sheet){
    cell_content_ = std::make_unique<EmptyImpl>();
}

Cell::~Cell() {
    cell_content_.reset(nullptr);
}

std::vector<Position> Cell::GetReferencedCells() const {
  std::vector<Position> referenced_cells;
    if(dynamic_cast<FormulaImpl*>(cell_content_.get())) {
        referenced_cells = dynamic_cast<FormulaImpl*>(cell_content_.get())->GetFormulaInterface()->GetReferencedCells();
     }
    return referenced_cells;
}

void Cell::Update() {
    for (CellInterface* c_c_t : cells_contain_this_) {
        Cell* current_cell = static_cast<Cell*>(c_c_t);
        current_cell->cells_contain_this_.erase(current_cell->cells_contain_this_.find(this));
    }
    cells_contain_this_.clear();

    std::vector<Position> new_cells_contain_this = GetReferencedCells();

    for (const Position& pos : new_cells_contain_this) {
        if (!sheet_.GetCell(pos)) {
            sheet_.SetCell(pos, "");
        }

       //std::cout << sheet_.GetCell(pos)->GetText() << std::endl;
       //auto it = cells_contain_this_.emplace(sheet_.GetCell(pos)).first;
     //  dynamic_cast<Cell*>(*it)->includes_cells_.insert(this);
    }
}

void Cell::InvalidateCache() {
    calculated_value_ = std::nullopt;
    for (CellInterface* c_c_t: cells_contain_this_) {
        dynamic_cast<Cell*>(c_c_t)->InvalidateCache();
    }
}

void Cell::Set(std::string text) {
    if(text.empty()) {
        cell_content_ = std::make_unique<EmptyImpl>();
        InvalidateCache();
        return;
    }
    if(text.size() == 1) {
        cell_content_ = std::make_unique<TextImpl>(text);
        InvalidateCache();
        return;
    }
    if(text[0] == '=') {

        std::unique_ptr<Impl> current_impl;
        try {
            current_impl = std::make_unique<FormulaImpl>(text.substr(1), sheet_);
        }  catch (std::exception&) {
            throw FormulaException("incorrect formula syntaxis");
        }
        CheckLoop(dynamic_cast<FormulaImpl*>(current_impl.get())->GetFormulaInterface()->GetReferencedCells(), this);
        std::swap(cell_content_, current_impl);
        Update();
        InvalidateCache();
        return;
    }
    cell_content_ = std::make_unique<TextImpl>(text);
    InvalidateCache();

}

void Cell::CheckLoop(const std::vector<Position>& formula_cells, const CellInterface* key_cell) {
    for (const Position& f_c : formula_cells) {

        auto fd = sheet_.GetCell(f_c);

        if (!fd) {
            sheet_.SetCell(f_c, "");
        }

        if (fd == this) {
            throw CircularDependencyException("loop reference detected");
        }

        CheckLoop(sheet_.GetCell(f_c)->GetReferencedCells(), key_cell);
    }
}

void Cell::Clear() {
    cell_content_.reset(nullptr);
    cell_content_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    return cell_content_->GetValue();
}

std::string Cell::GetText() const {
    return cell_content_->GetText();
}
