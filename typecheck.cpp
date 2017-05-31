#include "typecheck.hpp"

#define forall(iterator, listptr) \
  for(iterator = listptr->begin(); iterator != listptr->end(); iterator++) \



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

void TypeCheck::visitProgramNode(ProgramNode *node) {
  classTable = new ClassTable();
  node->visit_children(this);

  const VariableTable *programVarTable = classTable->at(currentClassName).members;
  const MethodTable *programMethodTable = classTable->at(currentClassName).methods;

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

void createClassInScopeHelper(ClassNode *node, TypeCheck *scope) {
  scope->currentClassName = node->identifier_1->name;
  scope->currentMethodTable = new MethodTable();
  scope->currentVariableTable = new VariableTable();
  scope->currentLocalOffset = 0;
  scope->currentMemberOffset = 0;
  scope->currentParameterOffset = 0;
}

ClassInfo &createClassInfoScopeHelper(ClassInfo &classInfo, IdentifierNode *secondID, TypeCheck *scope) {
  int byte_size = 4;
  classInfo.superClassName = (secondID) ? secondID->name : "";
  classInfo.methods = scope->currentMethodTable;
  classInfo.members = scope->currentVariableTable;
  classInfo.membersSize = byte_size * classInfo.members->size();
  return classInfo;
}

void TypeCheck::visitClassNode(ClassNode *node) {

  IdentifierNode *secondID = node->identifier_2;

  ClassInfo info;

  createClassInScopeHelper(node, this);

  if (secondID && !classTable->count(secondID->name)) {
    typeError(undefined_class);
    return;
  }
  createClassInfoScopeHelper(info, secondID, this);

  (*classTable)[currentClassName] = info;
  node->visit_children(this);

}

void returnStmntTypeError(MethodNode *node) {
  const ReturnStatementNode *returnStatement = node->methodbody->returnstatement;
  const BaseType nodeAST = node->type->basetype;
  if (!returnStatement && nodeAST != bt_none) {
    typeError(return_type_mismatch);
  }
  if (returnStatement && nodeAST != returnStatement->basetype && nodeAST != bt_none) {
    typeError(return_type_mismatch);
  }
  if (returnStatement && nodeAST == bt_object  &&
      node->type->objectClassName != returnStatement->objectClassName) {
    typeError(return_type_mismatch);
  }
  if (nodeAST == bt_none && returnStatement) {
    typeError(return_type_mismatch);
  }
}

void constructorErrorTypeError(MethodNode *node, TypeCheck *scope) {
  const std::string ID = node->identifier->name;
  const BaseType nodeAST = node->type->basetype;
  if (ID == scope->currentClassName && nodeAST != bt_none) {
    typeError(constructor_returns_type);
  }
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
  returnStmntTypeError(node);
  constructorErrorTypeError(node, this);

  for (std::list<ParameterNode *>::const_iterator iterator = node->parameter_list->begin();
       iterator != node->parameter_list->end(); ++iterator) {
    CompoundType paramInfo = {
        (*iterator)->basetype,
        (*iterator)->objectClassName = (*iterator)->type->objectClassName
    };
    info.parameters->push_back(paramInfo);
  }
  int keysize = info.variables->size() - info.parameters->size();
  info.localsSize = 4 * keysize;
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
  int byte_count = 4;
  bool btObj = node->basetype == bt_object;
  if (btObj) {
    node->objectClassName = node->type->objectClassName;
  }
  std::list<IdentifierNode *>::iterator identifier_iterator = node->identifier_list->begin();
  std::list<IdentifierNode *>::iterator identifier_iterator_fin = node->identifier_list->end();
  for (identifier_iterator; identifier_iterator != identifier_iterator_fin; ++identifier_iterator) {

    if (btObj && !(*classTable).count(node->type->objectClassName))
      typeError(undefined_class);

    CompoundType compoundDeclType = {
        node->basetype,
        (!btObj) ? "" : node->type->objectClassName
    };

    VariableInfo varInfo = {
        compoundDeclType,
        (!currentLocalOffset) ? currentMemberOffset : currentLocalOffset,
        byte_count
    };
    currentMemberOffset = (!currentLocalOffset) ? currentMemberOffset + byte_count : currentMemberOffset;
    currentLocalOffset = (!currentLocalOffset) ? currentLocalOffset : currentLocalOffset - byte_count;
    (*currentVariableTable)[(*identifier_iterator)->name] = varInfo;
  }
}

void TypeCheck::visitReturnStatementNode(ReturnStatementNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  const BaseType nodeAST = node->basetype;
  node->basetype = node->expression->basetype;
  node->objectClassName = nodeAST != bt_object ? node->expression->objectClassName : node->objectClassName;

}

void checkIfNotAnObject(bool &located, std::string &reference, std::string &myClass, AssignmentNode *node,
                        TypeCheck *scope) {
  const VariableTable *programVarTable = scope->classTable->at(scope->currentClassName).members;
  const MethodTable *programMethodTable = scope->classTable->at(scope->currentClassName).methods;
  std::string NAME = node->identifier_1->name;
  if ((*scope->currentVariableTable).count(NAME) && node->identifier_2 != NULL) {
    myClass = (*scope->currentVariableTable)[NAME].type.objectClassName;
    located = true;
    reference = myClass;
    if ((*scope->currentVariableTable)[NAME].type.baseType != bt_object) {
      typeError(not_object);
    }
  }
  if (!(*scope->currentVariableTable).count(NAME) && node->identifier_2 != NULL) {
    myClass = scope->currentClassName;
    while (myClass != "" && !located) {
      if ((*(*scope->classTable)[myClass].members).find(NAME) !=
          (*(*scope->classTable)[myClass].members).end()) {
        if ((*(*scope->classTable)[myClass].members)[NAME].type.baseType != bt_object)
          typeError(not_object);
        located = true;
        reference = myClass;
        break;
      }
      myClass = (*scope->classTable)[myClass].superClassName;
    }
  }
}

void checkIfUndefinedVariable(bool &located, std::string &reference, std::string &myClass, AssignmentNode *node,
                              TypeCheck *scope) {
  std::string NAME = node->identifier_1->name;
  if (!(*scope->currentVariableTable).count(NAME) && node->identifier_2 == NULL) {
    myClass = scope->currentClassName;
    while (myClass != "" && !located) {
      located = ((*(*scope->classTable)[myClass].members).count(NAME));
      if (located) {
        node->basetype = (*(*scope->classTable)[myClass].members)[NAME].type.baseType;
        node->objectClassName = (*(*scope->classTable)[myClass].members)[NAME].type.objectClassName;
      }
      myClass = (*scope->classTable)[myClass].superClassName;
    }
    if (!located)
      typeError(undefined_variable);
  }

  if (node->identifier_2 != NULL && !located)
    typeError(undefined_variable);
}

void TypeCheck::visitAssignmentNode(AssignmentNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  bool located = false;
  bool bufferIsA = false;

  const VariableTable *programVarTable = classTable->at(currentClassName).members;
  const MethodTable *programMethodTable = classTable->at(currentClassName).methods;
//  const MethodTable* programMethodTable = currentMethodTable;
//  const ClassTable::const_iterator className = (*classTable).find(programName);
//  const std::string programName = "Main" ;
  std::string NAME = node->identifier_1->name;
  std::string myClass;
  std::string reference;
  if ((*currentVariableTable).count(NAME) && node->identifier_2 == NULL) {
    node->basetype = (*currentVariableTable)[NAME].type.baseType;
    node->objectClassName = (*currentVariableTable)[NAME].type.objectClassName;
  }
  checkIfNotAnObject(located, reference, myClass, node, this);

  checkIfUndefinedVariable(located, reference, myClass, node, this);

  while (node->identifier_2 != NULL && reference != "") {
    if ((*(*classTable)[reference].members).count(node->identifier_2->name)) {
      bufferIsA = true;
      node->basetype = (*(*classTable)[reference].members)[node->identifier_2->name].type.baseType;
      node->objectClassName = (*(*classTable)[reference].members)[node->identifier_2->name].type.objectClassName;
      break;
    }
    reference = (*classTable)[reference].superClassName;
  }

  if (node->identifier_2 != NULL && !bufferIsA)
    typeError(undefined_member);
  if (node->basetype != node->expression->basetype)
    typeError(assignment_type_mismatch);

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
  node->visit_children(this);
  node->basetype = node->expression->basetype;
  node->objectClassName = node->expression->objectClassName;
}

void TypeCheck::visitPlusNode(PlusNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if (node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer)
    typeError(expression_type_mismatch);
  node->basetype = bt_integer;

}

void TypeCheck::visitMinusNode(MinusNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if (node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer)
    typeError(expression_type_mismatch);
  node->basetype = bt_integer;
}

void TypeCheck::visitTimesNode(TimesNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if (node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer)
    typeError(expression_type_mismatch);
  node->basetype = bt_integer;
}

void TypeCheck::visitDivideNode(DivideNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if (node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer)
    typeError(expression_type_mismatch);
  node->basetype = bt_integer;
}

void TypeCheck::visitLessNode(LessNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if (node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer)
    typeError(expression_type_mismatch);
  node->basetype = bt_boolean;
}

void TypeCheck::visitLessEqualNode(LessEqualNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if (node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer)
    typeError(expression_type_mismatch);
  node->basetype = bt_boolean;
}

void TypeCheck::visitEqualNode(EqualNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if (node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer)
    typeError(expression_type_mismatch);
  node->basetype = bt_boolean;
}

void TypeCheck::visitAndNode(AndNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if (node->expression_1->basetype != bt_boolean || node->expression_2->basetype != bt_boolean)
    typeError(expression_type_mismatch);
  node->basetype = bt_boolean;
}

void TypeCheck::visitOrNode(OrNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if (node->expression_1->basetype != bt_boolean || node->expression_2->basetype != bt_boolean)
    typeError(expression_type_mismatch);
  node->basetype = bt_boolean;
}

void TypeCheck::visitNotNode(NotNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if (node->expression->basetype != bt_boolean)
    typeError(expression_type_mismatch);
  node->basetype = bt_boolean;
}

void TypeCheck::visitNegationNode(NegationNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if (node->expression->basetype != bt_integer)
    typeError(expression_type_mismatch);
  node->basetype = bt_integer;
}

void checkForArgumentMismatch1(MethodCallNode *node, TypeCheck *scope) {
  node->basetype = (*scope->currentMethodTable)[node->identifier_1->name].returnType.baseType;
  node->objectClassName = (*scope->currentMethodTable)[node->identifier_1->name].returnType.objectClassName;

  if ((*(*scope->currentMethodTable)[node->identifier_1->name].parameters).size() != node->expression_list->size()) {
    typeError(argument_number_mismatch);
  }
  std::list<CompoundType>::iterator temp = (*scope->currentMethodTable)[node->identifier_1->name].parameters->begin();
  for (std::list<ExpressionNode *>::iterator expression = node->expression_list->begin();
       expression != node->expression_list->end(); ++temp, ++expression)
    if ((*temp).baseType != (*expression)->basetype)
      typeError(argument_type_mismatch);
}

void checkForArgumentMismatch2(std::string &reference, MethodCallNode *node, TypeCheck *scope) {
  if ((*(*(*scope->classTable)[reference].methods)[node->identifier_1->name].parameters).size() !=
      node->expression_list->size())
    typeError(argument_number_mismatch);
  std::list<CompoundType>::iterator temp = (*(*scope->classTable)[reference].methods)[node->identifier_1->name].parameters->begin();
  for (std::list<ExpressionNode *>::iterator expression = node->expression_list->begin();
       expression != node->expression_list->end(); ++temp, ++expression)
    if ((*temp).baseType != (*expression)->basetype)
      typeError(argument_type_mismatch);
}


void mutateAndCheckForNotAnObjectInMethodCall(bool& isLocated, std::string &reference, std::string &myClass, MethodCallNode *node, TypeCheck *scope) {
  if ((*scope->currentVariableTable).count(node->identifier_1->name) && node->identifier_2) {
    myClass = (*scope->currentVariableTable)[node->identifier_1->name].type.objectClassName;
    isLocated = true;
    reference = myClass;
  }
  if (!(*scope->currentVariableTable).count(node->identifier_1->name) && node->identifier_2) {
    while (myClass != "" && !isLocated) {
      if ((*(*scope->classTable)[myClass].members).count(node->identifier_1->name)) {
        if ((*(*scope->classTable)[myClass].members)[node->identifier_1->name].type.baseType != bt_object)
          typeError(not_object);
        isLocated = true;
        reference = (*(*scope->classTable)[myClass].members)[node->identifier_1->name].type.objectClassName;
        break;
      }
      myClass = (*scope->classTable)[myClass].superClassName;
    }
  }
}

void grabMyMethods(bool& bufferisA, std::string &reference, MethodCallNode *node, TypeCheck *scope) {
  while (reference != "" && node->identifier_2) {
    if ((*(*scope->classTable)[reference].methods).count(node->identifier_2->name)) {
      node->basetype = (*(*scope->classTable)[reference].methods)[node->identifier_2->name].returnType.baseType;
      node->objectClassName = (*(*scope->classTable)[reference].methods)[node->identifier_2->name].returnType.objectClassName;
      bufferisA = true;
      break;
    }
    reference = (*scope->classTable)[reference].superClassName;
  }
  if (!bufferisA)
    typeError(undefined_method);
}

void TypeCheck::visitMethodCallNode(MethodCallNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  bool isLocated = false;
  bool bufferisA = false;
  const VariableTable *programVarTable = classTable->at(currentClassName).members;
  const MethodTable *programMethodTable = classTable->at(currentClassName).methods;
//  const MethodTable* programMethodTable = currentMethodTable;
//  const ClassTable::const_iterator className = (*classTable).find(programName);
//  const std::string programName = "Main" ;
  std::string NAME = node->identifier_1->name;
  std::string myClass = currentClassName;
  std::string reference;


  if (node->identifier_2 && (*currentVariableTable).count(node->identifier_1->name) &&
      (*currentVariableTable)[node->identifier_1->name].type.baseType != bt_object) {
    typeError(not_object);
  }
  if (node->identifier_2) {
    mutateAndCheckForNotAnObjectInMethodCall(isLocated, reference, myClass, node, this);

    grabMyMethods(bufferisA, reference, node, this);
    if ((*(*(*classTable)[reference].methods)[node->identifier_2->name].parameters).size() !=
        node->expression_list->size())
      typeError(argument_number_mismatch);
    std::list<CompoundType>::iterator temp = (*(*classTable)[reference].methods)[node->identifier_2->name].parameters->begin();
    for (std::list<ExpressionNode *>::iterator expression = node->expression_list->begin();
         expression != node->expression_list->end(); ++temp, ++expression)
      if ((*temp).baseType != (*expression)->basetype)
        typeError(argument_type_mismatch);
  } else {
    if ((*currentMethodTable).find(node->identifier_1->name) != (*currentMethodTable).end()) {
      checkForArgumentMismatch1(node, this);

    } else {
      isLocated = false;
      myClass = currentClassName;
      grabMyMethods(bufferisA, myClass, node, this);
      checkForArgumentMismatch2(reference, node, this);

    }
  }
}

void isMemberNodeNotAnObject(bool &isLocated, std::string &reference, std::string &myClass, MemberAccessNode *node,
                             TypeCheck *scope) {
  VariableTable *varTable;
  if (!(*scope->currentVariableTable).count(node->identifier_1->name)) {

    while (myClass != "") {
      varTable = scope->classTable->at(myClass).members;
      if ((varTable)->find(node->identifier_1->name) !=
          varTable->end()) {
        if ((*(*scope->classTable)[myClass].members)[node->identifier_1->name].type.baseType != bt_object) {
          typeError(not_object);
        }
        isLocated = true;
        reference = (*(*scope->classTable)[myClass].members)[node->identifier_1->name].type.objectClassName;
        break;
      }
      myClass = (*scope->classTable)[myClass].superClassName;
    }

  } else {
    myClass = (*scope->currentVariableTable)[node->identifier_1->name].type.objectClassName;
    isLocated = true;
    reference = myClass;
    if ((*scope->currentVariableTable)[node->identifier_1->name].type.baseType != bt_object) {
      typeError(not_object);
    }
  }
}

void mutateAndCheckForUndefinedMember(bool &isBufferA, std::string &reference, std::string &myClass, MemberAccessNode *node,
                                      TypeCheck *scope) {
  IdentifierNode *secondID = node->identifier_2;

  while (reference != "") {
    if ((*(*scope->classTable)[reference].members).find(secondID->name) != (*(*scope->classTable)[reference].members).end()) {
      node->basetype = (*(*scope->classTable)[reference].members)[secondID->name].type.baseType;
      node->objectClassName = (*(*scope->classTable)[reference].members)[secondID->name].type.objectClassName;
      isBufferA = true;
      break;
    }
    reference = (*scope->classTable)[reference].superClassName;
  }
  if (!isBufferA)
    typeError(undefined_member);

}

void TypeCheck::visitMemberAccessNode(MemberAccessNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  std::string myClass = currentClassName;
  std::string reference;
  bool isLocated = false;
  bool isBufferA = false;
  std::string NAME = node->identifier_1->name;
  VariableTable *varTable;

  isMemberNodeNotAnObject(isLocated, reference, myClass, node, this);

  if (!isLocated)
    typeError(undefined_variable);

  mutateAndCheckForUndefinedMember(isBufferA, reference, myClass, node, this);
}

void mutateAndTypeCheckVariableNode(bool &isLocated, std::string &myClass, VariableNode *node, TypeCheck *scope) {
  std::string NAME = node->identifier->name;

  if (!(*scope->currentVariableTable).count(NAME)) {
    while ((myClass != "")) {
      if ((*(*scope->classTable)[myClass].members).count(NAME) && node->basetype == bt_object)
        node->objectClassName = (*(*scope->classTable)[myClass].members)[NAME].type.objectClassName;
      if ((*(*scope->classTable)[myClass].members).count(NAME) && node->basetype != bt_object) {
        node->basetype = (*(*scope->classTable)[myClass].members)[NAME].type.baseType;
        isLocated = true;
        break;
      }
      myClass = (*scope->classTable)[myClass].superClassName;
    }
  }
  if (!(*scope->currentVariableTable).count(NAME) && !isLocated)
    typeError(undefined_variable);
}

void TypeCheck::visitVariableNode(VariableNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  bool isLocated = false;
  std::string myClass = currentClassName;
  std::string NAME = node->identifier->name;

  mutateAndTypeCheckVariableNode(isLocated, myClass, node, this);

  if ((*currentVariableTable).count(NAME))
    node->basetype = (*currentVariableTable)[NAME].type.baseType;
  if ((*currentVariableTable).count(NAME) && node->basetype == bt_object)
    node->objectClassName = (*currentVariableTable)[NAME].type.objectClassName;
}

void TypeCheck::visitIntegerLiteralNode(IntegerLiteralNode *node) {
  // WRITEME: Replace with code if necessary
  node->basetype = bt_integer;
  node->visit_children(this);
}

void TypeCheck::visitBooleanLiteralNode(BooleanLiteralNode *node) {
  // WRITEME: Replace with code if necessary
  node->basetype = bt_boolean;
  node->visit_children(this);
}

void TypeCheck::visitNewNode(NewNode *node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  std::string NAME = node->identifier->name;
  if ((*classTable).find(NAME) == (*classTable).end())
    typeError(undefined_class);
  node->basetype = bt_object;
  node->objectClassName = NAME;
}

void TypeCheck::visitIntegerTypeNode(IntegerTypeNode *node) {
  // WRITEME: Replace with code if necessary
  node->basetype = bt_integer;
  node->visit_children(this);
}

void TypeCheck::visitBooleanTypeNode(BooleanTypeNode *node) {
  // WRITEME: Replace with code if necessary
  node->basetype = bt_boolean;
  node->visit_children(this);
}

void TypeCheck::visitObjectTypeNode(ObjectTypeNode *node) {
  // WRITEME: Replace with code if necessary
  std::string NAME = node->identifier->name;
  node->basetype = bt_object;
  node->objectClassName = NAME;
  node->visit_children(this);
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
