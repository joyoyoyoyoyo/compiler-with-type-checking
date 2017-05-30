#include "typecheck.hpp"

// Defines the function used to throw type errors. The possible
// type errors are defined as an enumeration in the header file.
void typeError(TypeErrorCode code) {
  switch (code) {
    case undefined_variable:
      std::cerr << "Undefined variable." << std::endl;
      break;
    case undefined_method:
      std::cerr << "Method does not exist." << std::endl;
      break;
    case undefined_class:
      std::cerr << "Class does not exist." << std::endl;
      break;
    case undefined_member:
      std::cerr << "Class member does not exist." << std::endl;
      break;
    case not_object:
      std::cerr << "Variable is not an object." << std::endl;
      break;
    case expression_type_mismatch:
      std::cerr << "Expression types do not match." << std::endl;
      break;
    case argument_number_mismatch:
      std::cerr << "Method called with incorrect number of arguments." << std::endl;
      break;
    case argument_type_mismatch:
      std::cerr << "Method called with argument of incorrect type." << std::endl;
      break;
    case while_predicate_type_mismatch:
      std::cerr << "Predicate of while loop is not boolean." << std::endl;
      break;
    case repeat_predicate_type_mismatch:
      std::cerr << "Predicate of repeat loop is not boolean." << std::endl;
      break;
    case if_predicate_type_mismatch:
      std::cerr << "Predicate of if statement is not boolean." << std::endl;
      break;
    case assignment_type_mismatch:
      std::cerr << "Left and right hand sides of assignment types mismatch." << std::endl;
      break;
    case return_type_mismatch:
      std::cerr << "Return statement type does not match declared return type." << std::endl;
      break;
    case constructor_returns_type:
      std::cerr << "Class constructor returns a value." << std::endl;
      break;
    case no_main_class:
      std::cerr << "The \"Main\" class was not found." << std::endl;
      break;
    case main_class_members_present:
      std::cerr << "The \"Main\" class has members." << std::endl;
      break;
    case no_main_method:
      std::cerr << "The \"Main\" class does not have a \"main\" method." << std::endl;
      break;
    case main_method_incorrect_signature:
      std::cerr << "The \"main\" method of the \"Main\" class has an incorrect signature." << std::endl;
      break;
  }
  exit(1);
}

// TypeCheck Visitor Functions: These are the functions you will
// complete to build the symbol table and type check the program.
// Not all functions must have code, many may be left empty.

template<class a_type, class b_type>
void set_type_expression(a_type *x, b_type *y) {

  x->basetype = y->basetype;
  x->objectClassName = y->objectClassName;
}

template<class a_type>
bool check_integer_type(a_type *node) {

  return (node->basetype == bt_integer) && !(node->objectClassName.compare("Integer"));
}


void TypeCheck::visitProgramNode(ProgramNode *node) {
  classTable = new ClassTable();
  node->visit_children(this);

  const VariableTable *programVarTable = classTable->at(currentClassName).members;
  const MethodTable *programMethodTable = classTable->at(currentClassName).methods;
//  const MethodTable* programMethodTable = currentMethodTable;
//  const ClassTable::const_iterator className = (*classTable).find(programName);
//  const std::string programName = "Main" ;


  if (!classTable->count(currentClassName)) {
    typeError(no_main_class);
    return;
  }
  if (programVarTable->size() != 0) {
    typeError(main_class_members_present);
    return;
  }
  if (!programMethodTable->count("main")) {
    typeError(no_main_method);
    return;
  }
  if (programMethodTable->at("main").returnType.baseType != bt_none) {
    typeError(main_method_incorrect_signature);
    return;
  }


}


void TypeCheck::visitClassNode(ClassNode *node) {

  IdentifierNode *secondID = node->identifier_2;

  ClassInfo info;

  // class name
  currentClassName = node->identifier_1->name;
  currentMethodTable = new MethodTable();
  currentVariableTable = new VariableTable();
  currentLocalOffset = 0;
  currentMemberOffset = 0;
  currentParameterOffset = 0;

  if (secondID && !classTable->count(secondID->name)) {
    typeError(undefined_class);
    return;
  }

  info.superClassName = (secondID) ? secondID->name : "";
  info.methods = this->currentMethodTable;
  info.members = this->currentVariableTable;
  info.membersSize = 4 * info.members->size();
  (*classTable)[currentClassName] = info;
  node->visit_children(this);

}

void TypeCheck::visitMethodNode(MethodNode *node) {
  // WRITEME: Replace with code if necessary
  MethodInfo info;

  currentParameterOffset = 12;
  currentLocalOffset = -4;
  currentVariableTable = new VariableTable();
  info.variables = currentVariableTable;
  info.parameters = new std::list<CompoundType>();

  node->visit_children(this);
  const BaseType nodeAST = node->type->basetype;
  const ReturnStatementNode *returnStatement = node->methodbody->returnstatement;
  const std::string ID = node->identifier->name;
  CompoundType returnType = {
      nodeAST,
      node->type->objectClassName
  };
  if (!returnStatement && nodeAST != bt_none) {
    typeError(return_type_mismatch);
  }
  if (returnStatement && nodeAST != returnStatement->basetype && nodeAST != bt_none) {
    typeError(return_type_mismatch);
  }
  if (returnStatement && nodeAST == bt_object && nodeAST != bt_none && node->type->objectClassName != returnStatement->objectClassName) {
    typeError(return_type_mismatch);
  }
  if (node->identifier->name == currentClassName && nodeAST != bt_none) {
    typeError(constructor_returns_type);
  }
  if (nodeAST == bt_none && returnStatement) {
    typeError(return_type_mismatch);
  }

  for (std::list<ParameterNode *>::const_iterator iterator = node->parameter_list->begin();
       iterator != node->parameter_list->end(); ++iterator) {
    CompoundType paramInfo = {
        (*iterator)->basetype,
        (*iterator)->objectClassName = (*iterator)->type->objectClassName
    };
    info.parameters->push_back(paramInfo);
  }
  int keysize = info.variables->size() - info.parameters->size();
  info.localsSize = 4*keysize;
  info.returnType = returnType;
  (*currentMethodTable)[ID] = info;
}

void TypeCheck::visitMethodBodyNode(MethodBodyNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
}

void TypeCheck::visitParameterNode(ParameterNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  const int byte_count = 4;
  const std::string ID = node->identifier->name;
  std::string nameASTnode;

  if (node->basetype != bt_object)
    nameASTnode = "";
  if (node->basetype == bt_object)
    nameASTnode = node->type->objectClassName;

  const BaseType nodeAST = node->type->basetype;
  node->basetype = nodeAST;
  node->objectClassName = nameASTnode;
  CompoundType methodBodyType = {
      node->basetype,
      node->basetype != bt_object ? "" : node->type->objectClassName
  };
  VariableInfo info = {
      methodBodyType,
      currentParameterOffset,
      byte_count
  };
  currentParameterOffset = currentParameterOffset + byte_count;
  (*currentVariableTable)[ID] = info;
}

void TypeCheck::visitDeclarationNode(DeclarationNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  node->basetype = node->type->basetype;
  if (node->basetype == bt_object) {
    node->objectClassName = node->type->objectClassName;
  }
  for (std::list<IdentifierNode *>::iterator it = node->identifier_list->begin();
       it != node->identifier_list->end(); ++it) {
    VariableInfo VI;
    CompoundType CT;
    CT.baseType = node->basetype;
    if (node->basetype == bt_object) {
      if ((*classTable).find(node->type->objectClassName) == (*classTable).end()) {
        typeError(undefined_class);
      }
      CT.objectClassName = node->type->objectClassName;
    } else {
      CT.objectClassName = "";
    }
    VI.type = CT;
    if (this->currentLocalOffset == 0) {
      VI.offset = this->currentMemberOffset;
      this->currentMemberOffset = this->currentMemberOffset + 4;

    } else {
      VI.offset = this->currentLocalOffset;
      this->currentLocalOffset = this->currentLocalOffset - 4;

    }
    VI.size = 4;

    (*currentVariableTable)[(*it)->name] = VI;
  }
}

void TypeCheck::visitReturnStatementNode(ReturnStatementNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);


  node->basetype = node->expression->basetype;
  if (node->basetype == bt_object) {
    node->objectClassName = node->expression->objectClassName;
  }

}

void TypeCheck::visitAssignmentNode(AssignmentNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  bool found;
  bool memfound;
  std::string className;
  std::string refClass;
  std::string NAME = node->identifier_1->name;
  const VariableTable *programVarTable = classTable->at(currentClassName).members;
  const MethodTable *programMethodTable = classTable->at(currentClassName).methods;
//  const MethodTable* programMethodTable = currentMethodTable;
//  const ClassTable::const_iterator className = (*classTable).find(programName);
//  const std::string programName = "Main" ;


  if (node->identifier_2 == NULL ) {
    if ((*currentVariableTable).count(NAME)) {
      node->basetype = (*currentVariableTable)[NAME].type.baseType;
      node->objectClassName = (*currentVariableTable)[NAME].type.objectClassName;
    } else {
      found = false;
      className = this->currentClassName;
      while (className != "" && !found) {
          found = ((*(*classTable)[className].members).count(NAME));
          if (found) {
          node->basetype = (*(*classTable)[className].members)[NAME].type.baseType;
          node->objectClassName = (*(*classTable)[className].members)[NAME].type.objectClassName;
        }
        className = (*classTable)[className].superClassName;
      }

      if (!found) {
        typeError(undefined_variable);
      }
    }
  }





  else { //member is identifier_1, variable identifier_2; see if member exists and the use classname to look up variable to set basetype
    found = false;
    memfound = false;

    if ((*currentVariableTable).count(NAME)) {
      className = (*currentVariableTable)[NAME].type.objectClassName;
      found = true;
      refClass = className;
      if ((*currentVariableTable)[NAME].type.baseType != bt_object) {
        typeError(not_object);
      }
    } else {
      className = this->currentClassName;
      while (className != "" && !found) {
        if ((*(*classTable)[className].members).find(NAME) !=
            (*(*classTable)[className].members).end()) {
          if ((*(*classTable)[className].members)[NAME].type.baseType != bt_object) {
            typeError(not_object);
          }
          found = true;
          refClass = className;
          break;
        }
        className = (*classTable)[className].superClassName;
      }
    }

    if (found == false) {
      typeError(undefined_variable);
    }

    while (refClass != "" && !memfound) {


      if ((*(*classTable)[refClass].members).count(node->identifier_2->name)) {
        memfound = true;
        node->basetype = (*(*classTable)[refClass].members)[node->identifier_2->name].type.baseType;
        node->objectClassName = (*(*classTable)[refClass].members)[node->identifier_2->name].type.objectClassName;
        break;
      }
      refClass = (*classTable)[refClass].superClassName;
    }

    if (memfound == false) {
      typeError(undefined_member);
    }

  }

  if (node->basetype != node->expression->basetype) {
    typeError(assignment_type_mismatch);
  }
}

void TypeCheck::visitCallNode(CallNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  node->basetype = node->methodcall->basetype;
}

void TypeCheck::visitIfElseNode(IfElseNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);

  if (node->expression->basetype != bt_boolean) {
    typeError(if_predicate_type_mismatch);
  }

  node->basetype = bt_boolean;
}

void TypeCheck::visitWhileNode(WhileNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if (node->expression->basetype != bt_boolean) {
    typeError(while_predicate_type_mismatch);
  }

  node->basetype = bt_boolean;
}

void TypeCheck::visitRepeatNode(RepeatNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if (node->expression->basetype != bt_boolean) {
    typeError(repeat_predicate_type_mismatch);
  }

}

void TypeCheck::visitPrintNode(PrintNode *node) {
  // WRITEME: Replace with code if necessary
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  node->basetype = node->expression->basetype;
  node->objectClassName = node->expression->objectClassName;
}

void TypeCheck::visitPlusNode(PlusNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);

  if (node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
    typeError(expression_type_mismatch);
  }
  node->basetype = bt_integer;

}

void TypeCheck::visitMinusNode(MinusNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);

  if (node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
    typeError(expression_type_mismatch);
  }
  node->basetype = bt_integer;
}

void TypeCheck::visitTimesNode(TimesNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);

  if (node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
    typeError(expression_type_mismatch);
  }
  node->basetype = bt_integer;
}

void TypeCheck::visitDivideNode(DivideNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);

  if (node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
    typeError(expression_type_mismatch);
  }
  node->basetype = bt_integer;
}

void TypeCheck::visitLessNode(LessNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);

  if (node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
    typeError(expression_type_mismatch);
  }

  node->basetype = bt_boolean;
}

void TypeCheck::visitLessEqualNode(LessEqualNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);

  if (node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
    typeError(expression_type_mismatch);
  }

  node->basetype = bt_boolean;
}

void TypeCheck::visitEqualNode(EqualNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);

  if (node->expression_1->basetype == bt_integer) {
    if (node->expression_2->basetype != bt_integer) {
      typeError(expression_type_mismatch);
    }
  } else if (node->expression_1->basetype == bt_boolean) {
    if (node->expression_2->basetype != bt_boolean) {
      typeError(expression_type_mismatch);
    }
  } else {
    typeError(expression_type_mismatch);
  }

  node->basetype = bt_boolean;
}

void TypeCheck::visitAndNode(AndNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);

  if (node->expression_1->basetype != bt_boolean || node->expression_2->basetype != bt_boolean) {
    typeError(expression_type_mismatch);
  }

  node->basetype = bt_boolean;
}

void TypeCheck::visitOrNode(OrNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);

  if (node->expression_1->basetype != bt_boolean || node->expression_2->basetype != bt_boolean) {
    typeError(expression_type_mismatch);
  }

  node->basetype = bt_boolean;
}

void TypeCheck::visitNotNode(NotNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);

  if (node->expression->basetype != bt_boolean) {
    typeError(expression_type_mismatch);
  }

  node->basetype = bt_boolean;
}

void TypeCheck::visitNegationNode(NegationNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);

  if (node->expression->basetype != bt_integer) {
    typeError(expression_type_mismatch);
  }
  node->basetype = bt_integer;
}

void TypeCheck::visitMethodCallNode(MethodCallNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  bool found;
  bool methodfound;
  std::string className;
  std::string refClass;
  if (node->identifier_2 == NULL) {
    if ((*currentMethodTable).find(node->identifier_1->name) != (*currentMethodTable).end()) {
      node->basetype = (*currentMethodTable)[node->identifier_1->name].returnType.baseType;
      node->objectClassName = (*currentMethodTable)[node->identifier_1->name].returnType.objectClassName;

      if ((*(*currentMethodTable)[node->identifier_1->name].parameters).size() != node->expression_list->size()) {
        typeError(argument_number_mismatch);
      }

      std::list<CompoundType>::iterator p = (*currentMethodTable)[node->identifier_1->name].parameters->begin();
      for (std::list<ExpressionNode *>::iterator e = node->expression_list->begin();
           e != node->expression_list->end(); ++p, ++e) {
        if ((*p).baseType != (*e)->basetype) {
          typeError(argument_type_mismatch);
        }
      }

    } else {
      found = false;
      className = this->currentClassName;
      while (className != "" && !found) {
        if ((*(*classTable)[className].methods).find(node->identifier_1->name) !=
            (*(*classTable)[className].methods).end()) {
          found = true;
          node->basetype = (*(*classTable)[className].methods)[node->identifier_1->name].returnType.baseType;
          node->objectClassName = (*(*classTable)[className].methods)[node->identifier_1->name].returnType.objectClassName;
          refClass = className;
          break;
        }
        className = (*classTable)[className].superClassName;
      }

      if (found == false) {
        typeError(undefined_method);
      }

      if ((*(*(*classTable)[refClass].methods)[node->identifier_1->name].parameters).size() !=
          node->expression_list->size()) {
        typeError(argument_number_mismatch);
      }

      std::list<CompoundType>::iterator p = (*(*classTable)[refClass].methods)[node->identifier_1->name].parameters->begin();
      for (std::list<ExpressionNode *>::iterator e = node->expression_list->begin();
           e != node->expression_list->end(); ++p, ++e) {
        if ((*p).baseType != (*e)->basetype) {
          typeError(argument_type_mismatch);
        }
      }

    }
  } else {

    found = false;
    methodfound = false;

    if ((*currentVariableTable).find(node->identifier_1->name) != (*currentVariableTable).end()) {
      className = (*currentVariableTable)[node->identifier_1->name].type.objectClassName;
      found = true;
      refClass = className;
      if ((*currentVariableTable)[node->identifier_1->name].type.baseType != bt_object) {
        typeError(not_object);
      }
    } else {
      className = this->currentClassName;
      while (className != "" && !found) {
        if ((*(*classTable)[className].members).find(node->identifier_1->name) !=
            (*(*classTable)[className].members).end()) {
          if ((*(*classTable)[className].members)[node->identifier_1->name].type.baseType != bt_object) {
            typeError(not_object);
          }
          found = true;
          refClass = (*(*classTable)[className].members)[node->identifier_1->name].type.objectClassName;
          break;
        }
        className = (*classTable)[className].superClassName;
      }
    }

    if (found == false) {
      typeError(undefined_variable);
    }

    while (refClass != "" && !methodfound) {
      if ((*(*classTable)[refClass].methods).find(node->identifier_2->name) !=
          (*(*classTable)[refClass].methods).end()) {
        node->basetype = (*(*classTable)[refClass].methods)[node->identifier_2->name].returnType.baseType;
        node->objectClassName = (*(*classTable)[refClass].methods)[node->identifier_2->name].returnType.objectClassName;
        methodfound = true;
        break;
      }
      refClass = (*classTable)[refClass].superClassName;
    }

    if (methodfound == false) {
      typeError(undefined_method);
    }

    if ((*(*(*classTable)[refClass].methods)[node->identifier_2->name].parameters).size() !=
        node->expression_list->size()) {
      typeError(argument_number_mismatch);
    }
    std::list<CompoundType>::iterator p = (*(*classTable)[refClass].methods)[node->identifier_2->name].parameters->begin();
    for (std::list<ExpressionNode *>::iterator e = node->expression_list->begin();
         e != node->expression_list->end(); ++p, ++e) {
      if ((*p).baseType != (*e)->basetype) {
        typeError(argument_type_mismatch);
      }
    }
  }
}

void TypeCheck::visitMemberAccessNode(MemberAccessNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  std::string className;
  std::string refClass;
  bool found = false;
  bool memfound = false;


  if ((*currentVariableTable).find(node->identifier_1->name) != (*currentVariableTable).end()) {
    className = (*currentVariableTable)[node->identifier_1->name].type.objectClassName;
    found = true;
    refClass = className;
    if ((*currentVariableTable)[node->identifier_1->name].type.baseType != bt_object) {
      typeError(not_object);
    }
  } else {
    className = this->currentClassName;
    while (className != "" && !found) {
      if ((*(*classTable)[className].members).find(node->identifier_1->name) !=
          (*(*classTable)[className].members).end()) {
        if ((*(*classTable)[className].members)[node->identifier_1->name].type.baseType != bt_object) {
          typeError(not_object);
        }
        found = true;
        refClass = (*(*classTable)[className].members)[node->identifier_1->name].type.objectClassName;
        break;
      }
      className = (*classTable)[className].superClassName;

    }
  }

  if (found == false) {
    typeError(undefined_variable);
  }

  //refclass empty
  while (refClass != "" && !memfound) {

    if ((*(*classTable)[refClass].members).find(node->identifier_2->name) != (*(*classTable)[refClass].members).end()) {
      node->basetype = (*(*classTable)[refClass].members)[node->identifier_2->name].type.baseType;
      node->objectClassName = (*(*classTable)[refClass].members)[node->identifier_2->name].type.objectClassName;
      memfound = true;
      break;
    }
    refClass = (*classTable)[refClass].superClassName;
  }


  if (memfound == false) {
    typeError(undefined_member);
  }
}

void TypeCheck::visitVariableNode(VariableNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);

  if ((*currentVariableTable).find(node->identifier->name) != (*currentVariableTable).end()) {
    node->basetype = (*currentVariableTable)[node->identifier->name].type.baseType;
    if (node->basetype == bt_object) {
      node->objectClassName = (*currentVariableTable)[node->identifier->name].type.objectClassName;
    }
    //}else if((*(*classTable)[currentClassName].members).find(node->identifier->name) != (*(*classTable)[currentClassName].members).end()){
//	 node->basetype =  (*(*classTable)[currentClassName].members)[node->identifier->name].type.baseType;
    // node->objectClassName =(*(*classTable)[currentClassName].members)[node->identifier->name].type.objectClassName;
  } else {
    bool found = false;
    std::string className = this->currentClassName;

    while ((className != "") && (!found)) {
      if ((*(*classTable)[className].members).find(node->identifier->name) !=
          (*(*classTable)[className].members).end()) {
        node->basetype = (*(*classTable)[className].members)[node->identifier->name].type.baseType;
        if (node->basetype == bt_object) {
          node->objectClassName = (*(*classTable)[className].members)[node->identifier->name].type.objectClassName;
        }
        found = true;
        break;
      }
      className = (*classTable)[className].superClassName;
    }

    if (!found) {
      typeError(undefined_variable);
    }

  }
}

void TypeCheck::visitIntegerLiteralNode(IntegerLiteralNode *node) {
  // WRITEME: Replace with code if necessary
  // WRITEME: Replace with code if necessary
  node->basetype = bt_integer;
  //node->objectClassName = "";
  node->visit_children(this);
}

void TypeCheck::visitBooleanLiteralNode(BooleanLiteralNode *node) {
  // WRITEME: Replace with code if necessary
  node->basetype = bt_boolean;
  //node->objectClassName = "";
  node->visit_children(this);
}

void TypeCheck::visitNewNode(NewNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if ((*classTable).find(node->identifier->name) == (*classTable).end()) {
    typeError(undefined_class);
  }
  //(*(*classTable)[node->identifier->name].methods)[node->identifier->name].parameters

  node->basetype = bt_object;
  node->objectClassName = node->identifier->name;
  //rule states doesn't produce none? when would it ever be none?
}

void TypeCheck::visitIntegerTypeNode(IntegerTypeNode *node) {
  // WRITEME: Replace with code if necessary
  node->basetype = bt_integer;
  //node->objectClassName = "";
  node->visit_children(this);
}

void TypeCheck::visitBooleanTypeNode(BooleanTypeNode *node) {
  // WRITEME: Replace with code if necessary
  // WRITEME: Replace with code if necessary
  node->basetype = bt_boolean;
  //node->objectClassName = "";
  node->visit_children(this);
}

void TypeCheck::visitObjectTypeNode(ObjectTypeNode *node) {
  // WRITEME: Replace with code if necessary
  node->basetype = bt_object;
  node->objectClassName = node->identifier->name;
  node->visit_children(this);
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitNoneNode(NoneNode *node) {
  // WRITEME: Replace with code if necessary
  node->basetype = bt_none;
  node->visit_children(this);
}

void TypeCheck::visitIdentifierNode(IdentifierNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);

}

void TypeCheck::visitIntegerNode(IntegerNode *node) {
  // WRITEME: Replace with code if necessary
  node->basetype = bt_integer;
  node->visit_children(this);
}


// The following functions are used to print the Symbol Table.
// They do not need to be modified at all.

std::string genIndent(int indent) {
  std::string string = std::string("");
  for (int i = 0; i < indent; i++)
    string += std::string(" ");
  return string;
}

std::string string(CompoundType type) {
  switch (type.baseType) {
    case bt_integer:
      return std::string("Integer");
    case bt_boolean:
      return std::string("Boolean");
    case bt_none:
      return std::string("None");
    case bt_object:
      return std::string("Object(") + type.objectClassName + std::string(")");
    default:
      return std::string("");
  }
}


void print(VariableTable variableTable, int indent) {
  std::cout << genIndent(indent) << "VariableTable {";
  if (variableTable.size() == 0) {
    std::cout << "}";
    return;
  }
  std::cout << std::endl;
  for (VariableTable::iterator it = variableTable.begin(); it != variableTable.end(); it++) {
    std::cout << genIndent(indent + 2) << it->first << " -> {" << string(it->second.type);
    std::cout << ", " << it->second.offset << ", " << it->second.size << "}";
    if (it != --variableTable.end())
      std::cout << ",";
    std::cout << std::endl;
  }
  std::cout << genIndent(indent) << "}";
}

void print(MethodTable methodTable, int indent) {
  std::cout << genIndent(indent) << "MethodTable {";
  if (methodTable.size() == 0) {
    std::cout << "}";
    return;
  }
  std::cout << std::endl;
  for (MethodTable::iterator it = methodTable.begin(); it != methodTable.end(); it++) {
    std::cout << genIndent(indent + 2) << it->first << " -> {" << std::endl;
    std::cout << genIndent(indent + 4) << string(it->second.returnType) << "," << std::endl;
    std::cout << genIndent(indent + 4) << it->second.localsSize << "," << std::endl;
    print(*it->second.variables, indent + 4);
    std::cout << std::endl;
    std::cout << genIndent(indent + 2) << "}";
    if (it != --methodTable.end())
      std::cout << ",";
    std::cout << std::endl;
  }
  std::cout << genIndent(indent) << "}";
}

void print(ClassTable classTable, int indent) {
  std::cout << genIndent(indent) << "ClassTable {" << std::endl;
  for (ClassTable::iterator it = classTable.begin(); it != classTable.end(); it++) {
    std::cout << genIndent(indent + 2) << it->first << " -> {" << std::endl;
    if (it->second.superClassName != "")
      std::cout << genIndent(indent + 4) << it->second.superClassName << "," << std::endl;
    print(*it->second.members, indent + 4);
    std::cout << "," << std::endl;
    print(*it->second.methods, indent + 4);
    std::cout << std::endl;
    std::cout << genIndent(indent + 2) << "}";
    if (it != --classTable.end())
      std::cout << ",";
    std::cout << std::endl;
  }
  std::cout << genIndent(indent) << "}" << std::endl;
}

void print(ClassTable classTable) {
  print(classTable, 0);
}
