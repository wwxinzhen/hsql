#include "sqlhelper.h"
#include <iostream>
#include <map>
#include <string>
#include <string.h>
#include <hsql/SQLParser.h>

int g_value = -1;
int g_flag = 0;
int g_key_value = -1;
int g_offset = 0;
namespace hsql {

  void printOperatorExpression(Expr* expr, uintmax_t numIndent);
  void printAlias(Alias* alias, uintmax_t numIndent);

  std::ostream& operator<<(std::ostream& os, const OperatorType& op);
  std::ostream& operator<<(std::ostream& os, const DatetimeField& op);

  std::string indent(uintmax_t numIndent) {
    return std::string(numIndent, '\t');
  }
  void inprint(int64_t val, uintmax_t numIndent) {
    std::cout << indent(numIndent).c_str() << val << "  " << std::endl;
  }
  void inprint(double val, uintmax_t numIndent) {
    std::cout << indent(numIndent).c_str() << val << std::endl;
  }
  void inprint(const char* val, uintmax_t numIndent) {
    std::cout << indent(numIndent).c_str() << val << std::endl;
  }
  void inprint(const char* val, const char* val2, uintmax_t numIndent) {
    std::cout << indent(numIndent).c_str() << val << "->" << val2 << std::endl;
  }
  void inprintC(char val, uintmax_t numIndent) {
    std::cout << indent(numIndent).c_str() << val << std::endl;
  }
  void inprint(const OperatorType& op, uintmax_t numIndent) {
    std::cout << indent(numIndent) << op << std::endl;
  }
  void inprint(const ColumnType& colType, uintmax_t numIndent) {
    std::cout << indent(numIndent) << colType << std::endl;
  }
  void inprint(const DatetimeField& colType, uintmax_t numIndent) {
    std::cout << indent(numIndent) << colType << std::endl;
  }

  void printTableRefInfo(TableRef* table, uintmax_t numIndent) {
    switch (table->type) {
    case kTableName:
      inprint(table->name, numIndent);
      if(table->schema)  {
        inprint("Schema", numIndent + 1);
        inprint(table->schema, numIndent + 2);
      }
      break;
    case kTableSelect:
      printSelectStatementInfo(table->select, numIndent);
      break;
    case kTableJoin:
      inprint("Join Table", numIndent);
      inprint("Left", numIndent + 1);
      printTableRefInfo(table->join->left, numIndent + 2);
      inprint("Right", numIndent + 1);
      printTableRefInfo(table->join->right, numIndent + 2);
      inprint("Join Condition", numIndent + 1);
      printExpression(table->join->condition, numIndent + 2);
      break;
    case kTableCrossProduct:
      for (TableRef* tbl : *table->list) printTableRefInfo(tbl, numIndent);
      break;
    }

    if (table->alias) {
      printAlias(table->alias, numIndent);
    }
  }

  void printAlias(Alias* alias, uintmax_t numIndent) {
    inprint("Alias", numIndent + 1);
    inprint(alias->name, numIndent + 2);

    if (alias->columns) {
      for (char* column : *(alias->columns)) {
        inprint(column, numIndent + 3);
      }
    }
  }

  void printOperatorExpression(Expr* expr, uintmax_t numIndent) 
  {
    if (expr == nullptr) 
    {
      inprint("null", numIndent);
      return;
    }

    //inprint(expr->opType, numIndent);

    //printExpression(expr->expr, numIndent + 1);
    printExpression(expr->expr, numIndent);             
    if (expr->expr2 != nullptr) 
    {
        //printExpression(expr->expr2, numIndent + 1);
        printExpression(expr->expr2, numIndent);        
    } 
    else if (expr->exprList != nullptr) 
    {
        for (Expr* e : *expr->exprList) 
        {
            //printExpression(e, numIndent + 1);
          printExpression(e, numIndent);
        }

    }
    if(g_value != -1)
    {
      g_key_value = g_value;
      g_value = -1;
    }
      
  }

  void printExpression(Expr* expr, uintmax_t numIndent) {
    if (!expr) 
    {
      return;
    }

    switch (expr->type) 
    {
    case kExprStar:
      inprint("*", numIndent);
      break;
    case kExprColumnRef:
      if(strstr(expr->name, "uid"))
        g_flag = 1;
      break;
    // case kExprTableColumnRef: inprint(expr->table, expr->name, numIndent); break;
    case kExprLiteralFloat:
      inprint(expr->fval, numIndent);
      break;
    case kExprLiteralInt:
      //inprint(expr->ival, numIndent);
      if(g_flag == 1)
      {
        g_value = expr->ival;
        g_flag = 0;
      }
      break;
    case kExprLiteralString:
      inprint(expr->name, numIndent);
      break;
    case kExprLiteralDate:
      inprint(expr->name, numIndent);
      break;
    case kExprLiteralNull:
      inprint("NULL", numIndent);
      break;
    case kExprLiteralInterval:
      inprint("INTERVAL", numIndent);
      inprint(expr->ival, numIndent + 1);
      inprint(expr->datetimeField, numIndent + 1);
      break;
    case kExprFunctionRef:
      inprint(expr->name, numIndent);
      for (Expr* e : *expr->exprList) printExpression(e, numIndent + 1);
      break;
    case kExprExtract:
      inprint("EXTRACT", numIndent);
      inprint(expr->datetimeField, numIndent + 1);
      printExpression(expr->expr, numIndent + 1);
      break;
    case kExprCast:
      inprint("CAST", numIndent);
      inprint(expr->columnType, numIndent + 1);
      printExpression(expr->expr, numIndent + 1);
      break;
    case kExprOperator:
      printOperatorExpression(expr, numIndent);
      break;
    case kExprSelect:
      printSelectStatementInfo(expr->select, numIndent);
      break;
    case kExprParameter:
      inprint(expr->ival, numIndent);
      break;
    case kExprArray:
      for (Expr* e : *expr->exprList) printExpression(e, numIndent + 1);
      break;
    case kExprArrayIndex:
      printExpression(expr->expr, numIndent + 1);
      inprint(expr->ival, numIndent);
      break;
    default:
      std::cerr << "Unrecognized expression type " << expr->type << std::endl;
      return;
    }
    if (expr->alias != nullptr) {
      inprint("Alias", numIndent + 1);
      inprint(expr->alias, numIndent + 2);
    }
  }

  void printOrderBy(const std::vector<OrderDescription*>* expr, uintmax_t numIndent) {
    if (!expr) return;
    for (const auto& order_description : *expr) {
      printExpression(order_description->expr, numIndent);
      if (order_description->type == kOrderAsc) {
          inprint("ascending", numIndent);
      }
      else {
          inprint("descending", numIndent);
      }
    }
  }
  void printSelectStatementInfo(const SelectStatement* stmt, uintmax_t numIndent) {
    if (stmt->whereClause != nullptr) 
    {
      printExpression(stmt->whereClause, 0);
    }

  }

  void printImportStatementInfo(const ImportStatement* stmt, uintmax_t numIndent) {
    inprint("ImportStatement", numIndent);
    inprint(stmt->filePath, numIndent + 1);
    switch (stmt->type) {
      case ImportType::kImportCSV:
        inprint("CSV", numIndent + 1);
        break;
      case ImportType::kImportTbl:
        inprint("TBL", numIndent + 1);
        break;
      case ImportType::kImportBinary:
        inprint("BINARY", numIndent + 1);
        break;
      case ImportType::kImportAuto:
        inprint("AUTO", numIndent + 1);
        break;
    }
    inprint(stmt->tableName, numIndent + 1);
  }

  void printExportStatementInfo(const ExportStatement* stmt, uintmax_t numIndent) {
    inprint("ExportStatement", numIndent);
    inprint(stmt->filePath, numIndent + 1);
    switch (stmt->type) {
      case ImportType::kImportCSV:
        inprint("CSV", numIndent + 1);
        break;
      case ImportType::kImportTbl:
        inprint("TBL", numIndent + 1);
        break;
      case ImportType::kImportBinary:
        inprint("BINARY", numIndent + 1);
        break;
      case ImportType::kImportAuto:
        inprint("AUTO", numIndent + 1);
        break;
    }
    inprint(stmt->tableName, numIndent + 1);
  }

  void printCreateStatementInfo(const CreateStatement* stmt, uintmax_t numIndent) {
    inprint("CreateStatement", numIndent);
    inprint(stmt->tableName, numIndent + 1);
    if (stmt->filePath) inprint(stmt->filePath, numIndent + 1);
  }

  void printInsertStatementInfo(const InsertStatement* stmt, uintmax_t numIndent) {
    int i = 0;
    for (char* col_name : *stmt->columns) 
    {
        ++i;
        if(strstr(col_name, "uid"))
          g_offset = i;
    }
    i = 0;
    switch (stmt->type) 
    {

      case kInsertValues:
        //inprint("Values", numIndent + 1);
        for (Expr* expr : *stmt->values) 
        {
          i++;
          if(i == g_offset)
            g_key_value = expr->ival;
          printExpression(expr, numIndent + 2);
        }
        break;
      case kInsertSelect:
        printSelectStatementInfo(stmt->select, numIndent + 1);
        break;
    }
  }

  void printTransactionStatementInfo(const TransactionStatement* stmt, uintmax_t numIndent) {
    inprint("TransactionStatement", numIndent);
    switch (stmt->command){
    case kBeginTransaction:
      inprint("BEGIN", numIndent + 1);
      break;
    case kCommitTransaction:
      inprint("COMMIT", numIndent + 1);
      break;
    case kRollbackTransaction:
      inprint("ROLLBACK", numIndent + 1);
      break;
    }
  }

  void printDeleteStatementInfo(const DeleteStatement* stmt, uintmax_t num_indent)
  {
    //printf("--debug: printDeleteStatementInfo %ld\n",stmt->expr->expr2->ival);
    g_key_value = stmt->expr->expr2->ival;
  }

  void printUpdateStatementInfo(const UpdateStatement* stmt, uintmax_t num_indent)
  {
    //printf("stmt->where->expr2->ival:%ld\n",stmt->where->expr2->ival);
    g_key_value = stmt->where->expr2->ival;
  }
  void printStatementInfo(const SQLStatement* stmt) {
    switch (stmt->type()) {
    case kStmtSelect:
      printSelectStatementInfo((const SelectStatement*) stmt, 0);
      break;
    case kStmtInsert:
      printInsertStatementInfo((const InsertStatement*) stmt, 0);
      break;
    case kStmtCreate:
      printCreateStatementInfo((const CreateStatement*) stmt, 0);
      break;
    case kStmtImport:
      printImportStatementInfo((const ImportStatement*) stmt, 0);
      break;
    case kStmtExport:
      printExportStatementInfo((const ExportStatement*) stmt, 0);
      break;
    case kStmtTransaction:
      printTransactionStatementInfo((const TransactionStatement*) stmt, 0);
      break;
    case kStmtDelete:
      printDeleteStatementInfo((const DeleteStatement*) stmt, 0);
    case kStmtUpdate:
      printUpdateStatementInfo((const UpdateStatement*) stmt, 0);
    default:
      break;
    }
  }

  std::ostream& operator<<(std::ostream& os, const OperatorType& op) {
    static const std::map<const OperatorType, const std::string> operatorToToken = {
      {kOpNone, "None"},
      {kOpBetween, "BETWEEN"},
      {kOpCase, "CASE"},
      {kOpCaseListElement, "CASE LIST ELEMENT"},
      {kOpPlus, "+"},
      {kOpMinus, "-"},
      {kOpAsterisk, "*"},
      {kOpSlash, "/"},
      {kOpPercentage, "%"},
      {kOpCaret, "^"},
      {kOpEquals, "="},
      {kOpNotEquals, "!="},
      {kOpLess, "<"},
      {kOpLessEq, "<="},
      {kOpGreater, ">"},
      {kOpGreaterEq, ">="},
      {kOpLike, "LIKE"},
      {kOpNotLike, "NOT LIKE"},
      {kOpILike, "ILIKE"},
      {kOpAnd, "AND"},
      {kOpOr, "OR"},
      {kOpIn, "IN"},
      {kOpConcat, "CONCAT"},
      {kOpNot, "NOT"},
      {kOpUnaryMinus, "-"},
      {kOpIsNull, "IS NULL"},
      {kOpExists, "EXISTS"}
    };

    const auto found = operatorToToken.find(op);
    if (found == operatorToToken.cend()) {
      return os << static_cast<uint64_t>(op);
    } else {
      return os << (*found).second;
    }
  }

  std::ostream& operator<<(std::ostream& os, const DatetimeField& datetime) {
    static const std::map<const DatetimeField, const std::string> operatorToToken = {
      {kDatetimeNone, "None"},
      {kDatetimeSecond, "SECOND"},
      {kDatetimeMinute, "MINUTE"},
      {kDatetimeHour, "HOUR"},
      {kDatetimeDay, "DAY"},
      {kDatetimeMonth, "MONTH"},
      {kDatetimeYear, "YEAR"}
    };

    const auto found = operatorToToken.find(datetime);
    if (found == operatorToToken.cend()) {
      return os << static_cast<uint64_t>(datetime);
    } else {
      return os << (*found).second;
    }
  }

} // namespace hsql


int da_sql_parsed(const char* query)
{
    hsql::SQLParserResult res;
    hsql::SQLParser::parse(query, &res);

    if(res.isValid())
    {
        hsql::printStatementInfo(res.getStatement(0));
        //printf("g_key_value = %d\n", g_key_value);
        return g_key_value;
    }
    else
    {
        fprintf(stderr,"Parsed error ! \n");
        return -1;
    }
}
