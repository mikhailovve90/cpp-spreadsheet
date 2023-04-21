#pragma once
#include "common.h"
#include "formula.h"
#include <string>
#include <unordered_set>
#include <optional>
class Impl {
public:
    virtual CellInterface::Value GetValue() const = 0;
    virtual std::string GetText() const = 0;
    virtual ~Impl(){};
};

class EmptyImpl : public Impl {
public:
    EmptyImpl() = default;
    CellInterface::Value GetValue() const override;
    std::string GetText() const override;
};

class TextImpl : public Impl {
public:
    TextImpl(std::string text);
    CellInterface::Value GetValue() const override;
    std::string GetText() const override;
private:
    std::string text_;
};

class FormulaImpl : public Impl {
public:
    FormulaImpl(std::string formula, SheetInterface& sheet);
    CellInterface::Value GetValue() const override;
    std::string GetText() const override;

    const FormulaInterface* GetFormulaInterface() const;

private:
    SheetInterface& sheet_;
    std::unique_ptr<FormulaInterface> formula_;

};

class Sheet;

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();
    Value GetValue() const override;
    std::string GetText() const override;

    void CheckLoop(const std::vector<Position>& formula_cells, const CellInterface* key_cell);
    void InvalidateCache();
    std::vector<Position> GetReferencedCells() const override;
    bool IsReferenced() const;
    void Update();
private:


    std::unique_ptr<Impl> cell_content_;
    // ссылка на таблицу в которой находится сама ячейка и созависимые
    Sheet& sheet_;
    // список ячеек которые содержит формула в нашей ячейке
    std::unordered_set<CellInterface*> includes_cells_;
    // список ячеек которые содержат нашу ячейку
    std::unordered_set<CellInterface*> cells_contain_this_;
    //
    mutable std::optional<Value> calculated_value_;
};
