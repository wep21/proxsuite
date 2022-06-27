//
// Copyright (c) 2022, INRIA
//
#include <qp/dense/wrapper.hpp>
#include <qp/sparse/wrapper.hpp>
#include <qp/status.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/eigen.h>
#include <pybind11/stl.h>

namespace pybind11 {
namespace detail {
template <typename T>
struct type_caster<tl::optional<T>> : optional_caster<tl::optional<T>> {};
template <>
struct type_caster<tl::nullopt_t> : public void_caster<tl::nullopt_t> {};

} // namespace detail
} // namespace pybind11

namespace proxsuite {
namespace qp {
using veg::isize;

namespace python {

template <typename T>
void exposeQpObjectDense(pybind11::module_ m) {

	::pybind11::class_<dense::QP<T>>(m, "QP")
			.def(::pybind11::init<i64, i64, i64>(),"Default constructor") // constructor
			.def_readwrite(
					"results",
					&dense::QP<T>::results,
					"class containing the solution or certificate of infeasibility, "
					"and "
					"information statistics in an info subclass.")
			.def_readwrite(
					"settings",
					&dense::QP<T>::settings,
					"class with settings option of the solver.")
			.def_readwrite(
					"model", &dense::QP<T>::model, "class containing the QP model")
			.def(
					"init",
					static_cast<void (dense::QP<T>::*)(dense::MatRef<T>,dense::VecRef<T>,dense::MatRef<T>,dense::VecRef<T>,
														dense::MatRef<T>,dense::VecRef<T>,dense::VecRef<T>,bool compute_preconditioner)>(&dense::QP<T>::init),
					"function for initialize the QP model.",
					pybind11::arg_v("H", tl::nullopt, "quadratic cost"),
					pybind11::arg_v("g", tl::nullopt, "linear cost"),
					pybind11::arg_v("A", tl::nullopt, "equality constraint matrix"),
					pybind11::arg_v("b", tl::nullopt, "equality constraint vector"),
					pybind11::arg_v("C", tl::nullopt, "inequality constraint matrix"),
					pybind11::arg_v(
							"l", tl::nullopt, "lower inequality constraint vector"),
					pybind11::arg_v(
							"u", tl::nullopt, "upper inequality constraint vector"),
					pybind11::arg_v("compute_preconditioner",true,"execute the preconditioner for reducing ill-conditioning and speeding up solver execution."))

			.def(
					"init",
					static_cast<void (dense::QP<T>::*)(const dense::SparseMat<T>,dense::VecRef<T>,const dense::SparseMat<T>,dense::VecRef<T>,
														const dense::SparseMat<T>,dense::VecRef<T>,dense::VecRef<T>,bool compute_preconditioner)>(&dense::QP<T>::init),
					"function for initialize the QP model.",
					pybind11::arg_v("H", tl::nullopt, "quadratic cost"),
					pybind11::arg_v("g", tl::nullopt, "linear cost"),
					pybind11::arg_v("A", tl::nullopt, "equality constraint matrix"),
					pybind11::arg_v("b", tl::nullopt, "equality constraint vector"),
					pybind11::arg_v("C", tl::nullopt, "inequality constraint matrix"),
					pybind11::arg_v(
							"l", tl::nullopt, "lower inequality constraint vector"),
					pybind11::arg_v(
							"u", tl::nullopt, "upper inequality constraint vector"),
					pybind11::arg_v("compute_preconditioner",true,"execute the preconditioner for reducing ill-conditioning and speeding up solver execution."))
			.def( 
					"solve",
					static_cast<void (dense::QP<T>::*)()>(&dense::QP<T>::solve),
					"function used for solving the QP problem, using default parameters.")
			.def(
					"solve",
					static_cast<void (dense::QP<T>::*)(tl::optional<dense::VecRef<T>> x,tl::optional<dense::VecRef<T>> y,tl::optional<dense::VecRef<T>> z)>(&dense::QP<T>::solve),
					"function used for solving the QP problem, when passing a warm start.")
			.def(
					"update",
					static_cast<void (dense::QP<T>::*)(tl::optional<dense::MatRef<T>>,tl::optional<dense::VecRef<T>>,tl::optional<dense::MatRef<T>>,tl::optional<dense::VecRef<T>>,
														tl::optional<dense::MatRef<T>>,tl::optional<dense::VecRef<T>>,tl::optional<dense::VecRef<T>>,bool update_preconditioner)>(&dense::QP<T>::update),
					"function used for updating matrix or vector entry of the model using dense matrix entries.",
					pybind11::arg_v("H", tl::nullopt, "quadratic cost"),
					pybind11::arg_v("g", tl::nullopt, "linear cost"),
					pybind11::arg_v("A", tl::nullopt, "equality constraint matrix"),
					pybind11::arg_v("b", tl::nullopt, "equality constraint vector"),
					pybind11::arg_v("C", tl::nullopt, "inequality constraint matrix"),
					pybind11::arg_v(
							"l", tl::nullopt, "lower inequality constraint vector"),
					pybind11::arg_v(
							"u", tl::nullopt, "upper inequality constraint vector"),
					pybind11::arg_v("update_preconditioner",true,"update the preconditioner considering new matrices entries for reducing ill-conditioning and speeding up solver execution. If set up to false, use previous derived preconditioner."))

			.def(
					"update",
					static_cast<void (dense::QP<T>::*)(const tl::optional<dense::SparseMat<T>>,tl::optional<dense::VecRef<T>>,const tl::optional<dense::SparseMat<T>>,tl::optional<dense::VecRef<T>>,
														const tl::optional<dense::SparseMat<T>>,tl::optional<dense::VecRef<T>>,tl::optional<dense::VecRef<T>>,bool update_preconditioner)>(&dense::QP<T>::update),
					"function used for updating matrix or vector entry of the model using sparse matrix entries.",
					pybind11::arg_v("H", tl::nullopt, "quadratic cost"),
					pybind11::arg_v("g", tl::nullopt, "linear cost"),
					pybind11::arg_v("A", tl::nullopt, "equality constraint matrix"),
					pybind11::arg_v("b", tl::nullopt, "equality constraint vector"),
					pybind11::arg_v("C", tl::nullopt, "inequality constraint matrix"),
					pybind11::arg_v(
							"l", tl::nullopt, "lower inequality constraint vector"),
					pybind11::arg_v(
							"u", tl::nullopt, "upper inequality constraint vector"),
					pybind11::arg_v("update_preconditioner",true,"update the preconditioner considering new matrices entries for reducing ill-conditioning and speeding up solver execution. If set up to false, use previous derived preconditioner."))

			.def(
					"update_proximal_parameters",
					&dense::QP<T>::update_proximal_parameters,
					"function used for updating proximal parameters of the solver. The "
					"user must settup back the solver before using solve method.",
					pybind11::arg_v("rho", tl::nullopt, "primal proximal parameter"),
					pybind11::arg_v(
							"mu_eq",
							tl::nullopt,
							"dual equality constraint proximal parameter"),
					pybind11::arg_v(
							"mu_in",
							tl::nullopt,
							"dual inequality constraint proximal parameter"))
			.def(
					"cleanup",
					&dense::QP<T>::cleanup,
					"function used for cleaning the workspace and result "
					"classes.");
}

template <typename T,typename I>
void exposeQpObjectSparse(pybind11::module_ m) {

	::pybind11::class_<sparse::QP<T,I>>(m, "QP")//,pybind11::module_local()
			.def(::pybind11::init<i64, i64, i64>()) // constructor
			.def(::pybind11::init<const sparse::SparseMat<bool, I>&,const sparse::SparseMat<bool,I>&,const sparse::SparseMat<bool,I>&>()) // constructor
			.def_readwrite(
					"results",
					&sparse::QP<T,I>::results,
					"class containing the solution or certificate of infeasibility, "
					"and "
					"information statistics in an info subclass.")
			.def_readwrite(
					"settings",
					&sparse::QP<T,I>::settings,
					"class with settings option of the solver.")
			.def(
					"init",
					&sparse::QP<T,I>::init,
					"function for initializing the model when passing sparse matrices in "
					"entry.",
					pybind11::arg_v("H", tl::nullopt, "quadratic cost"),
					pybind11::arg_v("g", tl::nullopt, "linear cost"),
					pybind11::arg_v("A", tl::nullopt, "equality constraint matrix"),
					pybind11::arg_v("b", tl::nullopt, "equality constraint vector"),
					pybind11::arg_v("C", tl::nullopt, "inequality constraint matrix"),
					pybind11::arg_v(
							"l", tl::nullopt, "lower inequality constraint vector"),
					pybind11::arg_v(
							"u", tl::nullopt, "upper inequality constraint vector"),
					pybind11::arg_v("compute_preconditioner",true,"execute the preconditioner for reducing ill-conditioning and speeding up solver execution."))
			
			.def(
					"update",
					&sparse::QP<T,I>::update,
					"function for updating the model when passing sparse matrices in "
					"entry.",
					pybind11::arg_v("H", tl::nullopt, "quadratic cost"),
					pybind11::arg_v("g", tl::nullopt, "linear cost"),
					pybind11::arg_v("A", tl::nullopt, "equality constraint matrix"),
					pybind11::arg_v("b", tl::nullopt, "equality constraint vector"),
					pybind11::arg_v("C", tl::nullopt, "inequality constraint matrix"),
					pybind11::arg_v(
							"l", tl::nullopt, "lower inequality constraint vector"),
					pybind11::arg_v(
							"u", tl::nullopt, "upper inequality constraint vector"),
					pybind11::arg_v("update_preconditioner",false,"update the preconditioner or re-use previous derived for reducing ill-conditioning and speeding up solver execution."))
			
			.def(
					"solve",
					static_cast<void (sparse::QP<T,I>::*)()>(&sparse::QP<T,I>::solve),
					"function used for solving the QP problem, using default parameters.")
			.def(
					"solve",
					static_cast<void (sparse::QP<T,I>::*)(tl::optional<sparse::VecRef<T>> x,tl::optional<sparse::VecRef<T>> y,tl::optional<sparse::VecRef<T>> z)>(&sparse::QP<T,I>::solve),
					"function used for solving the QP problem, when passing a warm start.")
			.def(
					"update_proximal_parameters",
					&sparse::QP<T,I>::update_proximal_parameters,
					"function used for updating proximal parameters of the solver. The "
					"user must settup back the solver before using solve method.",
					pybind11::arg_v("rho", tl::nullopt, "primal proximal parameter"),
					pybind11::arg_v(
							"mu_eq",
							tl::nullopt,
							"dual equality constraint proximal parameter"),
					pybind11::arg_v(
							"mu_in",
							tl::nullopt,
							"dual inequality constraint proximal parameter"))
			.def(
					"cleanup",
					&sparse::QP<T,I>::cleanup,
					"function used for cleaning the result "
					"class.");
}

} //namespace python

} // namespace qp
} // namespace proxsuite