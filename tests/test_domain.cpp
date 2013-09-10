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

#include <string>
#include <unittestpp.h>
#include <derplanner/compiler/assert.h>
#include <derplanner/compiler/s_expression.h>
#include <derplanner/compiler/ast.h>
#include <derplanner/compiler/domain.h>

using namespace plnnrc;

namespace
{
    std::string to_string(sexpr::node* root)
    {
        std::string result;

        if (root->type == sexpr::node_list)
        {
            result += "(";

            for (sexpr::node* n = root->first_child; n != 0; n = n->next_sibling)
            {
                result += to_string(n);

                if (!is_last(n))
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

    std::string node_to_string(ast::node* node)
    {
        #define PLNNRC_AST_NODE_INNER(NODE_ID) case ast::NODE_ID: return std::string(#NODE_ID);
        #define PLNNRC_AST_NODE_LEAF(NODE_ID)  case ast::NODE_ID: return std::string(#NODE_ID) + " " + to_string(node->s_expr);

        switch (node->type)
        {
        #include <derplanner/compiler/ast_node_tags.inl>
        default:
            plnnrc_assert(false);
            return std::string("<error>");
        }

        #undef PLNNRC_AST_NODE_INNER
        #undef PLNNRC_AST_NODE_LEAF
    }

    std::string to_string(ast::node* root, int level=0)
    {
        std::string result;

        for (int i = 0; i < level; ++i)
        {
            result += "    ";
        }

        result += node_to_string(root);

        for (ast::node* child = root->first_child; child != 0; child = child->next_sibling)
        {
            result += "\n" + to_string(child, level+1);
        }

        return result;
    }

    TEST(empty_domain)
    {
        char buffer[] = "(:domain)";
        sexpr::tree expr;
        expr.parse(buffer);
        ast::tree tree;
        ast::node* actual = ast::build_domain(tree, expr.root()->first_child);
        CHECK(actual);
        CHECK(!actual->first_child);
    }

    TEST(domain_ast_structure)
    {
        char buffer[] = \
"(:domain                         "
"    (:method (root)              "
"        ((start ?s) (finish ?f)) "
"        ((travel ?s ?f))         "
"    )                            "
")                                ";

        sexpr::tree expr;
        expr.parse(buffer);
        ast::tree tree;
        ast::node* actual_tree = ast::build_domain(tree, expr.root()->first_child);
        CHECK(actual_tree);
        std::string actual_str = to_string(actual_tree);

        const char* expected = \
"node_domain\n"
"    node_method\n"
"        node_atom root\n"
"        node_branch\n"
"            node_op_or\n"
"                node_op_and\n"
"                    node_atom start\n"
"                        node_term_variable ?s\n"
"                    node_atom finish\n"
"                        node_term_variable ?f\n"
"            node_tasklist\n"
"                node_atom travel\n"
"                    node_term_variable ?s\n"
"                    node_term_variable ?f";

        CHECK_EQUAL(expected, actual_str.c_str());
    }

    TEST(method_table)
    {
        char buffer[] = \
"(:domain                         "
"    (:method (m1 ?x ?y))         "
"    (:method (m2 ?z ?w))         "
")                                ";

        sexpr::tree expr;
        expr.parse(buffer);
        ast::tree tree;

        ast::node* root = ast::build_domain(tree, expr.root()->first_child);
        CHECK(root);

        ast::node* m1 = root->first_child;
        ast::node* m2 = root->first_child->next_sibling;

        CHECK_EQUAL(2u, tree.methods.count());
        CHECK_EQUAL(m1, tree.methods.find("m1"));
        CHECK_EQUAL(m2, tree.methods.find("m2"));
    }

    TEST(worldstate_ast_structure)
    {
        char buffer[] = \
"(:worldstate               "
"    (atomx (int) (double)) "
"    (atomy (const char *)) "
")                          ";

        sexpr::tree expr;
        expr.parse(buffer);
        ast::tree tree;
        ast::node* actual_tree = ast::build_worldstate(tree, expr.root()->first_child);
        CHECK(actual_tree);
        std::string actual_str = to_string(actual_tree);

        const char* expected = \
"node_worldstate\n"
"    node_atom atomx\n"
"        node_worldstate_type (int)\n"
"        node_worldstate_type (double)\n"
"    node_atom atomy\n"
"        node_worldstate_type (const char *)";

        CHECK_EQUAL(expected, actual_str.c_str());
    }

    TEST(worldstate_atom_table)
    {
        char buffer[] = \
"(:worldstate               "
"    (atomx (int) (int))    "
"    (atomy (double))       "
")                          ";

        sexpr::tree expr;
        expr.parse(buffer);
        ast::tree tree;
        ast::node* root = ast::build_worldstate(tree, expr.root()->first_child);

        ast::node* atomx = root->first_child;
        ast::node* atomy = root->first_child->next_sibling;

        CHECK_EQUAL(atomx, tree.ws_atoms.find("atomx"));
        CHECK_EQUAL(atomy, tree.ws_atoms.find("atomy"));
    }

    TEST(worldstate_type_table)
    {
        char buffer[] = \
"(:worldstate               "
"    (atomx (int) (int))    "
"    (atomy (double))       "
")                          ";

        sexpr::tree expr;
        expr.parse(buffer);
        ast::tree tree;
        ast::node* root = ast::build_worldstate(tree, expr.root()->first_child);

        ast::node* atomx = root->first_child;
        ast::node* atomy = root->first_child->next_sibling;

        ast::node* atomx_arg1 = atomx->first_child;
        ast::node* atomx_arg2 = atomx->first_child->next_sibling;
        ast::node* atomy_arg1 = atomy->first_child;

        int atomx_arg1_type_tag = ast::annotation<ast::ws_type_ann>(atomx_arg1)->type_tag;
        int atomx_arg2_type_tag = ast::annotation<ast::ws_type_ann>(atomx_arg2)->type_tag;
        int atomy_arg1_type_tag = ast::annotation<ast::ws_type_ann>(atomy_arg1)->type_tag;

        CHECK(atomx_arg1_type_tag == atomx_arg2_type_tag);
        CHECK(atomx_arg1_type_tag != atomy_arg1_type_tag);

        ast::node* ws_type_int = tree.ws_types.find("int");
        CHECK(ws_type_int);

        ast::node* ws_type_double = tree.ws_types.find("double");
        CHECK(ws_type_double);

        CHECK(atomx_arg1 == ws_type_int || atomx_arg2 == ws_type_int);
        CHECK(atomy_arg1 == ws_type_double);
    }

    TEST(type_inference)
    {
        char buffer[] = \
"(:worldstate                   "
"    (ax (type1) (type1))       "
"    (ay (type2) (type3))       "
"    (az (type2))               "
")                              "
"                               "
"(:domain                       "
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

        sexpr::tree expr;
        expr.parse(buffer);
        ast::tree tree;
        ast::build_worldstate(tree, expr.root()->first_child);
        ast::build_domain(tree, expr.root()->first_child->next_sibling);
        ast::infer_types(tree);

        const int type1 = 1;
        const int type2 = 2;
        const int type3 = 3;

        ast::node* m1_atom = tree.methods.find("m1")->first_child;
        ast::node* m2_atom = tree.methods.find("m2")->first_child;

        ast::node* m1_u = m1_atom->first_child;
        ast::node* m1_v = m1_atom->first_child->next_sibling;

        ast::node* m2_u = m2_atom->first_child;
        ast::node* m2_v = m2_atom->first_child->next_sibling;

        CHECK_EQUAL(type1, ast::annotation<ast::term_ann>(m1_u)->type_tag);
        CHECK_EQUAL(type3, ast::annotation<ast::term_ann>(m1_v)->type_tag);

        CHECK_EQUAL(type1, ast::annotation<ast::term_ann>(m2_u)->type_tag);
        CHECK_EQUAL(type2, ast::annotation<ast::term_ann>(m2_v)->type_tag);
    }
}
