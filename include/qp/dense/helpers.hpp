/**
 * @file helpers.hpp 
*/

#ifndef PROXSUITE_QP_DENSE_HELPERS_HPP
#define PROXSUITE_QP_DENSE_HELPERS_HPP
#include <tl/optional.hpp>
#include <qp/results.hpp>
#include <qp/settings.hpp>
#include <qp/status.hpp>
#include <qp/dense/fwd.hpp>
#include <chrono>

namespace proxsuite {
namespace qp {
namespace dense {

/////// SETUP ////////
template <typename T>
void compute_equality_constrained_initial_guess(
        Workspace<T>& qpwork,
		Settings<T>& qpsettings,
		Model<T>& qpmodel,
		Results<T>& qpresults){
    
    qpwork.rhs.setZero();
    qpwork.rhs.head(qpmodel.dim) = -qpwork.g_scaled;
    qpwork.rhs.segment(qpmodel.dim, qpmodel.n_eq) = qpwork.b_scaled;
    iterative_solve_with_permut_fact( //
            qpsettings,
            qpmodel,
            qpresults,
            qpwork,
            T(1),
            qpmodel.dim + qpmodel.n_eq);

    qpresults.x = qpwork.dw_aug.head(qpmodel.dim);
    qpresults.y = qpwork.dw_aug.segment(qpmodel.dim, qpmodel.n_eq);
    qpwork.dw_aug.setZero();
    qpwork.rhs.setZero();
}

template <typename T>
void compute_unconstrained_initial_guess(
        Workspace<T>& qpwork,
		Model<T>& qpmodel,
		Results<T>& qpresults){
    
    veg::dynstack::DynStackMut stack{
			veg::from_slice_mut,
			qpwork.ldl_stack.as_mut(),
	};
    linearsolver::dense::Ldlt<T> ldl_tmp;
    ldl_tmp.factorize(qpwork.kkt.topLeftCorner(qpmodel.dim, qpmodel.dim),stack);
    qpwork.rhs.setZero();
    qpwork.rhs.head(qpmodel.dim) = -qpwork.g_scaled;
    ldl_tmp.solve_in_place(qpwork.rhs.head(qpmodel.dim), stack);
    qpresults.x = qpwork.rhs.head(qpmodel.dim);
    qpwork.rhs.setZero();
}

template <typename T>
void setup_factorization(Workspace<T>& qpwork,
		Model<T>& qpmodel,
		Results<T>& qpresults){

        veg::dynstack::DynStackMut stack{
			veg::from_slice_mut,
			qpwork.ldl_stack.as_mut(),
	    };

        qpwork.kkt.topLeftCorner(qpmodel.dim, qpmodel.dim) = qpwork.H_scaled;
        qpwork.kkt.topLeftCorner(qpmodel.dim, qpmodel.dim).diagonal().array() +=
                qpresults.info.rho;
        qpwork.kkt.block(0, qpmodel.dim, qpmodel.dim, qpmodel.n_eq) =
                qpwork.A_scaled.transpose();
        qpwork.kkt.block(qpmodel.dim, 0, qpmodel.n_eq, qpmodel.dim) = qpwork.A_scaled;
        qpwork.kkt.bottomRightCorner(qpmodel.n_eq, qpmodel.n_eq).setZero();
        qpwork.kkt.diagonal()
                .segment(qpmodel.dim, qpmodel.n_eq)
                .setConstant(-qpresults.info.mu_eq);

        qpwork.ldl.factorize(qpwork.kkt, stack);
}

template <typename T>
void setup_equilibration(Workspace<T>& qpwork){

    QpViewBoxMut<T> qp_scaled{
			{from_eigen, qpwork.H_scaled},
			{from_eigen, qpwork.g_scaled},
			{from_eigen, qpwork.A_scaled},
			{from_eigen, qpwork.b_scaled},
			{from_eigen, qpwork.C_scaled},
			{from_eigen, qpwork.u_scaled},
			{from_eigen, qpwork.l_scaled}};

	veg::dynstack::DynStackMut stack{
			veg::from_slice_mut,
			qpwork.ldl_stack.as_mut(),
	};
	qpwork.ruiz.scale_qp_in_place(qp_scaled, stack);
	qpwork.correction_guess_rhs_g = infty_norm(qpwork.g_scaled);      
}

/*!
* Setup the linear solver and the parameters x, y and z (through warm starting or default values if warm_start=false in the settings)
*
* @param qpwork solver workspace
* @param qpsettings solver settings
* @param qpmodel solver model
* @param qpresults solver result 
*/
template <typename T>
void initial_guess(
		Workspace<T>& qpwork,
		Settings<T>& qpsettings,
		Model<T>& qpmodel,
		Results<T>& qpresults) {

    switch (qpsettings.initial_guess) {
				case InitialGuessStatus::UNCONSTRAINED_INITIAL_GUESS: {
                    compute_unconstrained_initial_guess(qpwork,qpmodel,qpresults);
                    break;
                }
                case InitialGuessStatus::EQUALITY_CONSTRAINED_INITIAL_GUESS:{
                    compute_equality_constrained_initial_guess(qpwork,qpsettings,qpmodel,qpresults);
                    break;
                }
    }  

}
/*!
* Setup the QP solver (the linear solver backend being dense).
*
* @param H quadratic cost input defining the QP model
* @param g linear cost input defining the QP model
* @param A equality constraint matrix input defining the QP model
* @param b equality constraint vector input defining the QP model
* @param C inequality constraint matrix input defining the QP model
* @param u lower inequality constraint vector input defining the QP model
* @param l lower inequality constraint vector input defining the QP model
* @param qpwork solver workspace
* @param qpsettings solver settings
* @param qpmodel solver model
* @param qpresults solver result 
*/
template <typename Mat, typename T>
void setup( //
		Mat const& H,
		VecRef<T> g,
		Mat const& A,
		VecRef<T> b,
		Mat const& C,
		VecRef<T> u,
		VecRef<T> l,
		Settings<T>& qpsettings,
		Model<T>& qpmodel,
		Workspace<T>& qpwork,
		Results<T>& qpresults) {

	qpmodel.H = Eigen::
			Matrix<T, Eigen::Dynamic, Eigen::Dynamic, to_eigen_layout(rowmajor)>(H);
	qpmodel.g = g;
	qpmodel.A = Eigen::
			Matrix<T, Eigen::Dynamic, Eigen::Dynamic, to_eigen_layout(rowmajor)>(A);
	qpmodel.b = b;
	qpmodel.C = Eigen::
			Matrix<T, Eigen::Dynamic, Eigen::Dynamic, to_eigen_layout(rowmajor)>(C);
	qpmodel.u = u;
	qpmodel.l = l;

	qpwork.H_scaled = qpmodel.H;
	qpwork.g_scaled = qpmodel.g;
	qpwork.A_scaled = qpmodel.A;
	qpwork.b_scaled = qpmodel.b;
	qpwork.C_scaled = qpmodel.C;
	qpwork.u_scaled = qpmodel.u;
	qpwork.l_scaled = qpmodel.l;

	qpwork.primal_feasibility_rhs_1_eq = infty_norm(qpmodel.b);
	qpwork.primal_feasibility_rhs_1_in_u = infty_norm(qpmodel.u);
	qpwork.primal_feasibility_rhs_1_in_l = infty_norm(qpmodel.l);
	qpwork.dual_feasibility_rhs_2 = infty_norm(qpmodel.g);

	switch (qpsettings.initial_guess) {
				case InitialGuessStatus::UNCONSTRAINED_INITIAL_GUESS: {
					setup_equilibration(qpwork);
    				setup_factorization(qpwork,qpmodel,qpresults);
                    break;
                }
                case InitialGuessStatus::EQUALITY_CONSTRAINED_INITIAL_GUESS:{
					setup_equilibration(qpwork);
    				setup_factorization(qpwork,qpmodel,qpresults);
                    break;
                }
                case InitialGuessStatus::COLD_START:{
					// keep solutions but restart workspace and results
					qpwork.cleanup();
					qpresults.cold_start();
					setup_equilibration(qpwork);
    				setup_factorization(qpwork,qpmodel,qpresults);
                    break;
                }
                case InitialGuessStatus::NO_INITIAL_GUESS:{
					qpwork.cleanup();
					qpresults.cleanup(); 
					setup_equilibration(qpwork);
    				setup_factorization(qpwork,qpmodel,qpresults);
                    break;
                }
				case InitialGuessStatus::WARM_START:{
					qpwork.cleanup();
					qpresults.cleanup(); 
					setup_equilibration(qpwork);
    				setup_factorization(qpwork,qpmodel,qpresults);
                    break;
                }
                case InitialGuessStatus::WARM_START_WITH_PREVIOUS_RESULT:{
                    // keep workspace and results solutions except statistics
					qpresults.cleanup_statistics();
                    break;
                }

	}

	initial_guess(qpwork, qpsettings, qpmodel, qpresults);

}

////// UPDATES ///////

/*!
* Update the proximal parameters of the results object.
*
* @param rho_new primal proximal parameter
* @param mu_eq_new dual equality proximal parameter
* @param mu_in_new dual inequality proximal parameter
* @param results solver result 
*/
template <typename T>
void update_proximal_parameters(
		Results<T>& results,
		tl::optional<T> rho_new,
		tl::optional<T> mu_eq_new,
		tl::optional<T> mu_in_new) {

	if (rho_new != tl::nullopt) {
		results.info.rho = rho_new.value();
	}
	if (mu_eq_new != tl::nullopt) {
		results.info.mu_eq = mu_eq_new.value();
		results.info.mu_eq_inv = T(1) / results.info.mu_eq;
	}
	if (mu_in_new != tl::nullopt) {
		results.info.mu_in = mu_in_new.value();
		results.info.mu_in_inv = T(1) / results.info.mu_in;
	}
}
/*!
* Warm start the results primal and dual variables.
*
* @param x_wm primal proximal parameter
* @param y_wm dual equality proximal parameter
* @param z_wm dual inequality proximal parameter
* @param results solver result 
* @param settings solver settings 
*/
template <typename T>
void warm_start(
		tl::optional<VecRef<T>> x_wm,
		tl::optional<VecRef<T>> y_wm,
		tl::optional<VecRef<T>> z_wm,
		Results<T>& results,
		Settings<T>& settings) {

	isize n_eq = results.y.rows();
	isize n_in = results.z.rows();
	if (n_eq!=0){
		if (n_in!=0){
			if(x_wm != tl::nullopt && y_wm != tl::nullopt && z_wm != tl::nullopt){
					results.x = x_wm.value().eval();
					results.y = y_wm.value().eval();
					results.z = z_wm.value().eval();
			}
		}else{
			// n_in= 0
			if(x_wm != tl::nullopt && y_wm != tl::nullopt){
					results.x = x_wm.value().eval();
					results.y = y_wm.value().eval();
			}
		}
	}else if (n_in !=0){
		// n_eq = 0
		if(x_wm != tl::nullopt && z_wm != tl::nullopt){
					results.x = x_wm.value().eval();
					results.z = z_wm.value().eval();
		}
	} else {
		// n_eq = 0 and n_in = 0
		if(x_wm != tl::nullopt ){
					results.x = x_wm.value().eval();
		}
	}	

	settings.initial_guess = InitialGuessStatus::WARM_START;

}
} // namespace dense
} // namespace qp
} // namespace proxsuite

#endif /* end of include guard PROXSUITE_QP_DENSE_HELPERS_HPP */