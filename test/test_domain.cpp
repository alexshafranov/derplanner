//
// Copyright (c) 2013 Alexander Shafranov shafranov@gmail.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#include <stdio.h>
#include <string>
#include <unittestpp.h>
#include <derplanner/compiler/assert.h>
#include <derplanner/compiler/s_expression.h>
#include <derplanner/compiler/ast.h>
#include "compiler/tree_tools.h"
#include "compiler/ast_domain.h"
#include "compiler/ast_worldstate.h"
#include "compiler/ast_infer.h"

using namespace plnnrc;

namespace
{
    std::string to_string(sexpr::Node* root)
    {
        std::string result;

        if (sexpr::is_list(root))
        {
            result += "(";

            for (sexpr::Node* n = root->first_child; n != 0; n = n->next_sibling)
            {
                result += to_string(n);

                if (!plnnrc::is_last(n))
                {
                    result += " ";
                }
            }

            result += ")";
        }
        else
        {
            result = root->token;
        }

        return result;
    }

    std::string node_to_string(ast::Node* Node)
    {
        #define PLNNRC_AST_NODE(NODE_ID) case ast::node_##NODE_ID: return std::string("node_" #NODE_ID);
        #define PLNNRC_AST_NODE_WITH_TOKEN(NODE_ID)  case ast::node_##NODE_ID: return std::string("node_" #NODE_ID) + " " + to_string(Node->s_expr);

        switch (Node->type)
        {
        #include <derplanner/compiler/ast_node_tags.inl>
        default:
            plnnrc_assert(false);
            return std::string("<error>");
        }

        #undef PLNNRC_AST_NODE_WITH_TOKEN
        #undef PLNNRC_AST_NODE
    }

    std::string to_string(ast::Node* root, int level=0)
    {
        std::string result;

        for (int i = 0; i < level; ++i)
        {
            result += "    ";
        }

        result += node_to_string(root);

        for (ast::Node* child = root->first_child; child != 0; child = child->next_sibling)
        {
            result += "\n" + to_string(child, level+1);
        }

        return result;
    }

    TEST(empty_domain)
    {
        char buffer[] = "(:domain (test))";
        sexpr::Tree expr;
        expr.parse(buffer);
        ast::Tree tree;
        ast::Node* actual = ast::build_domain(tree, expr.root()->first_child);
        CHECK(actual);
        CHECK(!actual->first_child->next_sibling);
    }

    TEST(domain_ast_structure)
    {
        char buffer[] = \
"(:domain (test)                  "
"    (:method (root)              "
"        ((start ?s) (finish ?f)) "
"        ((travel ?s ?f))         "
"    )                            "
")                                ";

        sexpr::Tree expr;
        expr.parse(buffer);
        ast::Tree tree;
        ast::Node* actual_tree = ast::build_domain(tree, expr.root()->first_child);
        CHECK(actual_tree);
        std::string actual_str = to_string(actual_tree);

        const char* expected = \
"node_domain\n"
"    node_namespace (test)\n"
"    node_method\n"
"        node_atom root\n"
"        node_branch\n"
"            node_op_or\n"
"                node_op_and\n"
"                    node_atom start\n"
"                        node_term_variable ?s\n"
"                    node_atom finish\n"
"                        node_term_variable ?f\n"
"            node_task_list\n"
"                node_atom travel\n"
"                    node_term_variable ?s\n"
"                    node_term_variable ?f";

        CHECK_EQUAL(expected, actual_str.c_str());
    }

    TEST(declared_operator)
    {
        char buffer[] = \
"(:domain (test)                        "
"   (:method (root x y z)               "
"       ()                              "
"       ((!declared x y) (!stub z))     "
"   )                                   "
"                                       "
"   (:operator (!declared x y)          "
"       (:delete (removed x))           "
"       (:add (added y))                "
"   )                                   "
")                                      ";

        sexpr::Tree expr;
        expr.parse(buffer);
        ast::Tree tree;
        ast::Node* actual_tree = ast::build_domain(tree, expr.root()->first_child);
        CHECK(actual_tree);
        std::string actual_str = to_string(actual_tree);

        const char* expected = \
"node_domain\n"
"    node_namespace (test)\n"
"    node_method\n"
"        node_atom root\n"
"            node_term_variable x\n"
"            node_term_variable y\n"
"            node_term_variable z\n"
"        node_branch\n"
"            node_op_or\n"
"                node_op_and\n"
"            node_task_list\n"
"                node_atom !declared\n"
"                    node_term_variable x\n"
"                    node_term_variable y\n"
"                node_atom !stub\n"
"                    node_term_variable z\n"
"    node_operator\n"
"        node_atom !declared\n"
"            node_term_variable x\n"
"            node_term_variable y\n"
"        node_delete_list\n"
"            node_atom removed\n"
"                node_term_variable x\n"
"        node_add_list\n"
"            node_atom added\n"
"                node_term_variable y";

        CHECK_EQUAL(expected, actual_str.c_str());
    }

    TEST(declared_and_stub_operators)
    {
        char buffer[] = \
"(:domain (test)                        "
"   (:method (root x y z)               "
"       ()                              "
"       ((!declared x y) (!stub z))     "
"   )                                   "
"                                       "
"   (:operator (!declared x y)          "
"       (:delete (removed x))           "
"       (:add (added y))                "
"   )                                   "
")                                      ";

        sexpr::Tree expr;
        expr.parse(buffer);
        ast::Tree tree;
        ast::Node* actual_tree = ast::build_domain(tree, expr.root()->first_child);
        CHECK(actual_tree);

        ast::Node* declared_operator = tree.operators.find("!declared");
        CHECK(declared_operator);

        const char* expected_declared = \
"node_operator\n"
"    node_atom !declared\n"
"        node_term_variable x\n"
"        node_term_variable y\n"
"    node_delete_list\n"
"        node_atom removed\n"
"            node_term_variable x\n"
"    node_add_list\n"
"        node_atom added\n"
"            node_term_variable y";

        CHECK_EQUAL(expected_declared, to_string(declared_operator).c_str());

        ast::Node* stub_operator = tree.operators.find("!stub");
        CHECK(stub_operator);

        const char* expected_stub = \
"node_operator\n"
"    node_atom !stub\n"
"        node_term_variable z\n"
"    node_delete_list\n"
"    node_add_list";

        CHECK_EQUAL(expected_stub, to_string(stub_operator).c_str());
    }

    TEST(method_table)
    {
        char buffer[] = \
"(:domain (test)                  "
"    (:method (m1 ?x ?y))         "
"    (:method (m2 ?z ?w))         "
")                                ";

        sexpr::Tree expr;
        expr.parse(buffer);
        ast::Tree tree;

        ast::Node* root = ast::build_domain(tree, expr.root()->first_child);
        CHECK(root);

        ast::Node* m1 = root->first_child->next_sibling;
        ast::Node* m2 = m1->next_sibling;

        CHECK_EQUAL(2u, tree.methods.count());
        CHECK_EQUAL(m1, tree.methods.find("m1"));
        CHECK_EQUAL(m2, tree.methods.find("m2"));
    }

    TEST(worldstate_ast_structure)
    {
        char buffer[] = \
"(:worldstate (test)        "
"    (atomx (int) (double)) "
"    (atomy (const char *)) "
")                          ";

        sexpr::Tree expr;
        expr.parse(buffer);
        ast::Tree tree;
        ast::Node* actual_tree = ast::build_worldstate(tree, expr.root()->first_child);
        CHECK(actual_tree);
        std::string actual_str = to_string(actual_tree);

        const char* expected = \
"node_worldstate\n"
"    node_namespace (test)\n"
"    node_atom atomx\n"
"        node_worldstate_type (int)\n"
"        node_worldstate_type (double)\n"
"    node_atom atomy\n"
"        node_worldstate_type (const char *)";

        CHECK_EQUAL(expected, actual_str.c_str());
    }

    TEST(worldstate_call_terms_definition)
    {
        char buffer[] = \
"(:worldstate (test)                                    "
"   (:function (function_name (int) (double)) -> (int)) "
")                                                      ";

        sexpr::Tree expr;
        expr.parse(buffer);

        ast::Tree tree;
        ast::Node* actual_tree = ast::build_worldstate(tree, expr.root()->first_child);
        CHECK(actual_tree);
        std::string actual_str = to_string(actual_tree);

        const char* expected = \
"node_worldstate\n"
"    node_namespace (test)\n"
"    node_function\n"
"        node_atom function_name\n"
"            node_worldstate_type (int)\n"
"            node_worldstate_type (double)\n"
"        node_worldstate_type (int)";

        CHECK_EQUAL(expected, actual_str.c_str());

        CHECK_EQUAL(2u, tree.ws_types.count());

        ast::Node* ws_type_int = tree.ws_types.find("int");
        CHECK(ws_type_int);

        ast::Node* ws_type_double = tree.ws_types.find("double");
        CHECK(ws_type_double);

        CHECK_EQUAL(0u, tree.ws_atoms.count());
        CHECK(tree.ws_funcs.find("function_name") != 0);
    }

    TEST(worldstate_atom_table)
    {
        char buffer[] = \
"(:worldstate (test)        "
"    (atomx (int) (int))    "
"    (atomy (double))       "
")                          ";

        sexpr::Tree expr;
        expr.parse(buffer);
        ast::Tree Tree;
        ast::Node* root = ast::build_worldstate(Tree, expr.root()->first_child);

        ast::Node* atomx = root->first_child->next_sibling;
        ast::Node* atomy = atomx->next_sibling;

        CHECK_EQUAL(atomx, Tree.ws_atoms.find("atomx"));
        CHECK_EQUAL(atomy, Tree.ws_atoms.find("atomy"));
    }

    TEST(worldstate_type_table)
    {
        char buffer[] = \
"(:worldstate (test)        "
"    (atomx (int) (int))    "
"    (atomy (double))       "
")                          ";

        sexpr::Tree expr;
        expr.parse(buffer);
        ast::Tree tree;
        ast::Node* root = ast::build_worldstate(tree, expr.root()->first_child);

        ast::Node* atomx = root->first_child->next_sibling;
        ast::Node* atomy = atomx->next_sibling;

        ast::Node* atomx_arg1 = atomx->first_child;
        ast::Node* atomx_arg2 = atomx->first_child->next_sibling;
        ast::Node* atomy_arg1 = atomy->first_child;

        int atomx_arg1_type_tag = ast::annotation<ast::WS_Type_Ann>(atomx_arg1)->type_tag;
        int atomx_arg2_type_tag = ast::annotation<ast::WS_Type_Ann>(atomx_arg2)->type_tag;
        int atomy_arg1_type_tag = ast::annotation<ast::WS_Type_Ann>(atomy_arg1)->type_tag;

        CHECK(atomx_arg1_type_tag == atomx_arg2_type_tag);
        CHECK(atomx_arg1_type_tag != atomy_arg1_type_tag);

        ast::Node* ws_type_int = tree.ws_types.find("int");
        CHECK(ws_type_int);

        ast::Node* ws_type_double = tree.ws_types.find("double");
        CHECK(ws_type_double);

        CHECK(atomx_arg1 == ws_type_int || atomx_arg2 == ws_type_int);
        CHECK(atomy_arg1 == ws_type_double);
    }

    TEST(type_inference)
    {
        char buffer[] = \
"(:worldstate (test)            "
"    (ax (type1) (type1))       "
"    (ay (type2) (type3))       "
"    (az (type2))               "
")                              "
"                               "
"(:domain (test)                "
"   (:method (m1 ?u ?v)         "
"       ((ax ?t ?u) (ay ?k ?v)) "
"       ((m2 ?t ?k))            "
"   )                           "
"                               "
"   (:method (m2 ?u ?v)         "
"       ((az ?v))               "
"       ((!o ?u ?v))            "
"   )                           "
")                              ";

        sexpr::Tree expr;
        expr.parse(buffer);
        ast::Tree Tree;
        ast::build_worldstate(Tree, expr.root()->first_child);
        ast::build_domain(Tree, expr.root()->first_child->next_sibling);
        ast::seed_types(Tree);
        ast::infer_types(Tree);

        const int type1 = 1;
        const int type2 = 2;
        const int type3 = 3;

        ast::Node* m1_atom = Tree.methods.find("m1")->first_child;
        ast::Node* m2_atom = Tree.methods.find("m2")->first_child;

        ast::Node* m1_u = m1_atom->first_child;
        ast::Node* m1_v = m1_atom->first_child->next_sibling;

        ast::Node* m2_u = m2_atom->first_child;
        ast::Node* m2_v = m2_atom->first_child->next_sibling;

        CHECK_EQUAL(type1, ast::annotation<ast::Term_Ann>(m1_u)->type_tag);
        CHECK_EQUAL(type3, ast::annotation<ast::Term_Ann>(m1_v)->type_tag);

        CHECK_EQUAL(type1, ast::annotation<ast::Term_Ann>(m2_u)->type_tag);
        CHECK_EQUAL(type2, ast::annotation<ast::Term_Ann>(m2_v)->type_tag);
    }
}
