#include "../sigmaplus_prover.h"
#include "../sigmaplus_verifier.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(lelantus_sigma_tests)

BOOST_AUTO_TEST_CASE(one_out_of_N)
{
    int N = 16;
    int n = 4;
    int index = 0;

    int m = (int)(log(N) / log(n));;

    secp_primitives::GroupElement g;
    g.randomize();
    std::vector<secp_primitives::GroupElement> h_gens;
    h_gens.resize(n * m);
    for(int i = 0; i < n * m; ++i ){
        h_gens[i].randomize();
    }
    secp_primitives::Scalar v, r;
    v.randomize();
    r.randomize();
    lelantus::SigmaPlusProver<secp_primitives::Scalar,secp_primitives::GroupElement> prover(g,h_gens, n, m);

    std::vector<secp_primitives::GroupElement> commits;
    for(int i = 0; i < N; ++i){
        if(i == index){
            secp_primitives::GroupElement c;
            secp_primitives::Scalar zero(uint64_t(0));
            c = lelantus::LelantusPrimitives<secp_primitives::Scalar,secp_primitives::GroupElement>::double_commit(g, zero, h_gens[1], v, h_gens[0], r);
            commits.push_back(c);

        }
        else{
            commits.push_back(secp_primitives::GroupElement());
            commits[i].randomize();
        }
    }

    lelantus::SigmaPlusProof<secp_primitives::Scalar,secp_primitives::GroupElement> proof;

    prover.proof(commits, index, v, r, proof);

    lelantus::SigmaPlusVerifier<secp_primitives::Scalar,secp_primitives::GroupElement> verifier(g, h_gens, n, m);
    BOOST_CHECK(verifier.verify(commits, proof));

}

BOOST_AUTO_TEST_CASE(one_out_of_N_batch)
{
    int N = 16;
    int n = 4;
    int m = (int)(log(N) / log(n));;

    secp_primitives::GroupElement g;
    g.randomize();
    std::vector<secp_primitives::GroupElement> h_gens;
    h_gens.resize(n * m);
    for(int i = 0; i < n * m; ++i ){
        h_gens[i].randomize();
    }

    lelantus::SigmaPlusProver<secp_primitives::Scalar,secp_primitives::GroupElement> prover(g,h_gens, n, m);

    std::vector<secp_primitives::GroupElement> commits;
    std::vector<secp_primitives::Scalar> serials;
    std::vector<secp_primitives::Scalar> v_;
    std::vector<secp_primitives::Scalar> r_;
    std::vector<int> indexes;
    for(int i = 0; i < N; ++i){
        if(i % 2){
            secp_primitives::Scalar s, r;
            s.randomize();
            serials.push_back(s);
            r.randomize();
            r_.push_back(r);
            secp_primitives::Scalar v(1);
            v_.push_back(v);
            indexes.push_back(i);

            secp_primitives::GroupElement c;
            c = lelantus::LelantusPrimitives<secp_primitives::Scalar,secp_primitives::GroupElement>::double_commit(g, s, h_gens[1], v, h_gens[0], r);
            commits.push_back(c);
        }
        else{
            commits.push_back(secp_primitives::GroupElement());
            commits[i].randomize();
        }
    }

    std::vector<lelantus::SigmaPlusProof<secp_primitives::Scalar,secp_primitives::GroupElement>> proofs;
    proofs.reserve(serials.size());


    std::vector<secp_primitives::Scalar> rA, rB, rC, rD;
    rA.resize(N);
    rB.resize(N);
    rC.resize(N);
    rD.resize(N);
    std::vector<std::vector<secp_primitives::Scalar>> sigma;
    sigma.resize(N);
    std::vector<std::vector<secp_primitives::Scalar>> Tk, Pk, Yk;
    Tk.resize(N);
    Pk.resize(N);
    Yk.resize(N);
    std::vector<std::vector<secp_primitives::Scalar>> a;
    a.resize(N);
    for(int i = 0; i < serials.size(); ++i){
        lelantus::SigmaPlusProof<secp_primitives::Scalar,secp_primitives::GroupElement> proof;
        proofs.push_back(proof);
        std::vector<secp_primitives::GroupElement> commits_;
        secp_primitives::GroupElement gs = g * serials[i].negate();
        for(int j = 0; j < commits.size(); ++j){
            GroupElement c_ = commits[j] + gs ;
            commits_.push_back(c_);
        }
        rA[i].randomize();
        rB[i].randomize();
        rC[i].randomize();
        rD[i].randomize();
        Tk[i].resize(m);
        Pk[i].resize(m);
        Yk[i].resize(m);
        a[i].resize(n * m);
        prover.sigma_commit(commits_, indexes[i], rA[i], rB[i], rC[i], rD[i], a[i], Tk[i], Pk[i], Yk[i], sigma[i], proofs[i]);
    }
    secp_primitives::Scalar x;
    lelantus::LelantusPrimitives<Scalar, GroupElement>::generate_Lelantus_challange(proofs, x);

    for(int i = 0; i < serials.size(); ++i)
        prover.sigma_response(sigma[i], a[i], rA[i], rB[i], rC[i], rD[i], v_[i], r_[i], Tk[i], Pk[i], x, proofs[i]);

    lelantus::SigmaPlusVerifier<secp_primitives::Scalar,secp_primitives::GroupElement> verifier(g, h_gens, n, m);
    BOOST_CHECK(verifier.batchverify(commits, x, serials, proofs));
}

BOOST_AUTO_TEST_SUITE_END()