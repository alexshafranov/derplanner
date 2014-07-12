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

#include <string.h>
#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/s_expression.h"
#include "derplanner/compiler/ast.h"
#include "error_tools.h"
#include "tree_tools.h"
#include "ast_term.h"
#include "tokens.h"
#include "ast_logical_expression.h"

namespace plnnrc {
namespace ast {

Node* build_logical_expression(Tree& ast, sexpr::Node* s_expr)
{
    PLNNRC_CHECK_NODE(root, ast.make_node(node_op_and, s_expr));

    // s_expr is operator or atom
    if (s_expr->first_child && sexpr::is_symbol(s_expr->first_child))
    {
        PLNNRC_CHECK_NODE(child, build_logical_expression_recursive(ast, s_expr));
        append_child(root, child);
    }
    // list of operators (i.e. s_expr is 'and' by default)
    else
    {
        for (sexpr::Node* c_expr = s_expr->first_child; c_expr != 0; c_expr = c_expr->next_sibling)
        {
            PLNNRC_CONTINUE(expect_type(ast, c_expr, sexpr::node_list, root));
            PLNNRC_CHECK_NODE(child, build_logical_expression_recursive(ast, c_expr));
            append_child(root, child);
        }
    }

    return root;
}

Node* build_logical_op(Tree& ast, sexpr::Node* s_expr, Node_Type op_type)
{
    PLNNRC_CHECK_NODE(root, ast.make_node(op_type, s_expr));

    for (sexpr::Node* c_expr = s_expr->first_child->next_sibling; c_expr != 0; c_expr = c_expr->next_sibling)
    {
        PLNNRC_CONTINUE(expect_type(ast, c_expr, sexpr::node_list, root));
        PLNNRC_CHECK_NODE(child, build_logical_expression_recursive(ast, c_expr));
        append_child(root, child);
    }

    return root;
}

Node* build_comparison_op(Tree& ast, sexpr::Node* s_expr, Node_Type op_type)
{
    sexpr::Node* name_expr = s_expr->first_child;
    PLNNRC_CHECK_NODE(root, ast.make_node(op_type, name_expr));

    for (sexpr::Node* c_expr = s_expr->first_child->next_sibling; c_expr != 0; c_expr = c_expr->next_sibling)
    {
        PLNNRC_CHECK_NODE(child, build_term(ast, c_expr));
        append_child(root, child);
    }

    return root;
}

Node* build_logical_expression_recursive(Tree& ast, sexpr::Node* s_expr)
{
    PLNNRC_RETURN(expect_child_type(ast, s_expr, sexpr::node_symbol));
    sexpr::Node* c_expr = s_expr->first_child;

    if (is_token(c_expr, token_and))
    {
        return build_logical_op(ast, s_expr, node_op_and);
    }

    if (is_token(c_expr, token_or))
    {
        return build_logical_op(ast, s_expr, node_op_or);
    }

    if (is_token(c_expr, token_not))
    {
        return build_logical_op(ast, s_expr, node_op_not);
    }

    if (is_token(c_expr, token_eq))
    {
        return build_comparison_op(ast, s_expr, node_op_eq);
    }

    if (is_token(c_expr, token_ne))
    {
        return build_comparison_op(ast, s_expr, node_op_ne);
    }

    if (is_token(c_expr, token_le))
    {
        return build_comparison_op(ast, s_expr, node_op_le);
    }

    if (is_token(c_expr, token_ge))
    {
        return build_comparison_op(ast, s_expr, node_op_ge);
    }

    if (is_token(c_expr, token_lt))
    {
        return build_comparison_op(ast, s_expr, node_op_lt);
    }

    if (is_token(c_expr, token_gt))
    {
        return build_comparison_op(ast, s_expr, node_op_gt);
    }

    if (ast.ws_funcs.find(c_expr->token))
    {
        return build_call_term(ast, s_expr);
    }

    return build_atom(ast, s_expr);
}

Node* convert_to_nnf(Tree& ast, Node* root)
{
    Node* r = root;

    for (Node* p = root; p != 0; p = preorder_traversal_next(r, p))
    {
        if (is_op_not(p))
        {
            Node* c = p->first_child;
            plnnrc_assert(c != 0);

            if (is_logical_op(c))
            {
                // eliminate double negation: (not (not x)) = (x)
                if (is_op_not(c))
                {
                    Node* x = c->first_child;
                    plnnrc_assert(x != 0);
                    detach_node(x);

                    // get rid of 'p' and 'c'
                    if (p->parent)
                    {
                        insert_child(p, x);
                        detach_node(p);
                    }

                    // update root of the tree if we're replacing it
                    if (r == p)
                    {
                        r = x;
                    }

                    p = x;
                }
                // de-morgan's law: (not (or x y)) = (and (not x) (not y)); (not (and x y)) = (or (not x) (not y))
                else
                {
                    p->type = is_op_and(c) ? node_op_or : node_op_and;
                    Node* after = c;

                    for (Node* x = c->first_child; x != 0;)
                    {
                        Node* next_x = x->next_sibling;

                        detach_node(x);

                        PLNNRC_CHECK_NODE(new_not, ast.make_node(node_op_not));
                        append_child(new_not, x);
                        insert_child(after, new_not);
                        after = new_not;

                        x = next_x;
                    }

                    detach_node(c);
                }
            }
        }
    }

    return r;
}

void flatten(Node* root)
{
    plnnrc_assert(root != 0);

    for (Node* p = root; p != 0; p = preorder_traversal_next(root, p))
    {
        if (is_logical_op(p) && !is_op_not(p))
        {
            for (;;)
            {
                bool collapsed = false;

                for (Node* n = p->first_child; n != 0;)
                {
                    Node* next_n = n->next_sibling;

                    // collapse: (and x (and y) z) -> (and x y z); (or x (or y) z) -> (or x y z)
                    if (n->type == p->type)
                    {
                        Node* after = n;

                        for (Node* c = n->first_child; c != 0;)
                        {
                            Node* next_c = c->next_sibling;
                            detach_node(c);
                            insert_child(after, c);
                            after = c;
                            c = next_c;
                        }

                        detach_node(n);
                        collapsed = true;
                    }

                    n = next_n;
                }

                if (!collapsed)
                {
                    break;
                }
            }
        }
    }
}

namespace
{
    inline bool is_literal(Node* root)
    {
        plnnrc_assert(root != 0);
        return is_op_not(root) || is_term_call(root) || is_atom(root) || is_comparison_op(root);
    }

    bool is_conjunction_of_literals(Node* root)
    {
        plnnrc_assert(root != 0);

        if (!is_op_and(root))
        {
            return false;
        }

        for (Node* n = root->first_child; n != 0; n = n->next_sibling)
        {
            if (!is_literal(n))
            {
                return false;
            }
        }

        return true;
    }

    inline Node* find_first(Node* root, Node_Type type)
    {
        for (Node* n = root->first_child; n != 0; n = n->next_sibling)
        {
            if (n->type == type)
            {
                return n;
            }
        }

        return 0;
    }

    bool distribute_and(Tree& ast, Node* node_and)
    {
        plnnrc_assert(node_and && is_op_and(node_and));

        Node* node_or = find_first(node_and, node_op_or);
        plnnrc_assert(node_or != 0);

        Node* after = node_and;

        for (Node* or_child = node_or->first_child; or_child != 0;)
        {
            Node* next_or_child = or_child->next_sibling;
            Node* new_and = ast.make_node(node_op_and);

            if (!new_and)
            {
                return false;
            }

            for (Node* and_child = node_and->first_child; and_child != 0;)
            {
                Node* next_and_child = and_child->next_sibling;

                if (and_child != node_or)
                {
                    Node* and_child_clone = ast.clone_subtree(and_child);

                    if (!and_child_clone)
                    {
                        return false;
                    }

                    append_child(new_and, and_child_clone);
                }
                else
                {
                    detach_node(or_child);
                    append_child(new_and, or_child);
                }

                and_child = next_and_child;
            }

            flatten(new_and);
            insert_child(after, new_and);
            after = new_and;

            or_child = next_or_child;
        }

        detach_node(node_and);

        return true;
    }

    Node* convert_to_dnf_or(Tree& ast, Node* root)
    {
        plnnrc_assert(root && is_op_or(root));

        for (;;)
        {
            bool done = true;

            for (Node* n = root->first_child; n != 0;)
            {
                Node* next_n = n->next_sibling;

                if (!is_literal(n) && !is_conjunction_of_literals(n))
                {
                    done = false;

                    PLNNRC_CHECK(distribute_and(ast, n));
                }

                n = next_n;
            }

            if (done)
            {
                break;
            }
        }

        return root;
    }
}

Node* convert_to_dnf(Tree& ast, Node* root)
{
    plnnrc_assert(root);

    PLNNRC_CHECK_NODE(nnf_root, convert_to_nnf(ast, root));
    PLNNRC_CHECK_NODE(new_root, ast.make_node(node_op_or));

    append_child(new_root, nnf_root);
    flatten(new_root);

    return convert_to_dnf_or(ast, new_root);
}

}
}
