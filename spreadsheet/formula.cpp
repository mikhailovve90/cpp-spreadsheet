#include "formula.h"
#include "cell.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#DIV/0!";
}

double GetValue(const SheetInterface& sheet, Position position) {
    Cell::Value value;
    double result;
    //Пробуем получить позицию, если позиция невалидна, то
    try {
        if(sheet.GetCell(position)) {
            value = sheet.GetCell(position)->GetValue();
        } else {
           return 0; //Ежели получили нулптр значит значение этой ячейки пусто тоесть 0
        }
    }  catch (InvalidPositionException&) {
        throw FormulaError(FormulaError::Category::Ref);
    }

    //Пробуем получить значение
    if (std::holds_alternative<double>(value)) {
        result = std::get<double>(value);
    } else if (std::holds_alternative<FormulaError>(value)) {
        throw std::get<FormulaError>(value);
    } else {
        if (std::get<std::string>(value) == "") {
            result = 0;
        } else {
            try {
                result = std::stod(std::get<std::string>(value));
            }  catch (std::invalid_argument&) {
                throw FormulaError(FormulaError::Category::Value);
            }
        }
    }
    return result;
}

namespace {
class Formula : public FormulaInterface {
public:
// Реализуйте следующие методы:
    explicit Formula(std::string expression) : ast_(ParseFormulaAST(expression)){
    }

    Value Evaluate(const SheetInterface& sheet) const override {
        std::function<double(Position)> get_value_function = [&](Position position) {
            return GetValue(sheet, position);;
        };

        Value value;
        try {
            value = ast_.Execute(get_value_function);
        }  catch (FormulaError& error) {
            value = error;
        }
        return value;
    }

    std::string GetExpression() const override {
        std::ostringstream os_formula;
        ast_.PrintFormula(os_formula);
        return os_formula.str();
    }

    virtual std::vector<Position> GetReferencedCells() const override {
        std::forward_list<Position> list = ast_.GetCells();
        std::set<Position> referenced_cells_set;
        std::vector<Position> referenced_cells_vec;
        //Чтобы не было повторок и было упорядочено заполняю сет
        for(auto pos{list.begin()};pos != list.end(); ++pos) {
            referenced_cells_set.insert(*pos);
        }

        referenced_cells_vec.resize(referenced_cells_set.size());
        //мувлю все элементы сета в вектор по очереди???
        std::move(referenced_cells_set.begin(), referenced_cells_set.end(), referenced_cells_vec.begin());

        return referenced_cells_vec;
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    try {
        ParseFormulaAST(expression);
    }
    catch (...) {
          throw FormulaException("Form");
    }
    return std::make_unique<Formula>(std::move(expression));
}
