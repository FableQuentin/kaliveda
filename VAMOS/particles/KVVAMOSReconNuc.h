//Created by KVClassFactory on Thu Sep 13 10:51:51 2012
//Author: Guilain ADEMARD

#ifndef __KVVAMOSRECONNUC_H
#define __KVVAMOSRECONNUC_H

#include "KVReconstructedNucleus.h"
#include "KVVAMOSCodes.h"
#include "KVVAMOSReconTrajectory.h"
#include "KVVAMOS.h"

class KVVAMOSDetector;
class KVVAMOSReconGeoNavigator;

class KVVAMOSReconNuc : public KVReconstructedNucleus
{
	private:

		KVVAMOSCodes fCodes; //Focal plane Pos. recon., calib. and ident. codes
	protected:

		KVVAMOSReconTrajectory fRT;             //handles trajectory reconstruction data
		Double_t               fStripFoilEloss; // calculated energy lost in the stripping foil
		Float_t                fToF;            //Time of Flight value in ns
		Float_t                fFlightDist;     //Flight distance in cm

		Float_t               *fDetE;           //!array with the corrected energy lost in each detector of fDetList

		virtual void CalibrateFromDetList();
		virtual void CalibrateFromTracking();
		virtual void MakeDetectorList();
		virtual Bool_t SetCorrectedFlightDistanceAndTime( Double_t tof,  KVVAMOSDetector *start, KVVAMOSDetector *stop=NULL );
		virtual Bool_t SetFlightDistance( KVVAMOSDetector *start, KVVAMOSDetector *stop=NULL );
		KVVAMOSReconGeoNavigator *GetNavigator();

   	public:

		KVVAMOSReconNuc();
   		KVVAMOSReconNuc (const KVVAMOSReconNuc&) ;
   		virtual ~KVVAMOSReconNuc();
   		virtual void Copy (TObject&) const;
		void init();

		virtual Float_t  GetCorrectedT_HF( Float_t tof, Float_t dist);
		virtual void     Calibrate();
		virtual Bool_t   CheckTrackingCoherence();
		virtual void     Clear(Option_t * t = "");
 		virtual void     GetAnglesFromStoppingDetector(Option_t * opt = "random");
		        Float_t  GetDeltaPath( KVVAMOSDetector *det ) const;
				using    KVReconstructedNucleus::GetEnergy;
				Int_t    GetDetectorIndex( const Char_t *det_label ) const;
				Float_t  GetEnergy( const Char_t *det_label )        const;
				Float_t  GetEnergyAfter( const Char_t *det_label )   const;
				Float_t  GetEnergyBefore( const Char_t *det_label )  const;
		        Double_t GetMassOverQ()                              const;
                Float_t  GetPath(KVVAMOSDetector *start, KVVAMOSDetector *stop=NULL)      const;
                Float_t  GetPath(const Char_t *start_label, const Char_t *stop_label="") const;
		        Double_t GetRealA()                                  const;
		        Double_t GetRealAoverQ()                             const;
 		virtual void     Identify();
		virtual void     ReconstructTrajectory();
		virtual void     ReconstructFPtraj();
		virtual void     ReconstructFPtrajByFitting();
		virtual void     ReconstructLabTraj();

		virtual void     Propagate();
		virtual void     SetFlightDistanceAndTime();


		//-------------- inline methods -----------------//
		static  Double_t         CalculateEnergy( Int_t Z, Int_t A, Int_t Q, Float_t Brho );
		static  Double_t         CalculateEnergy( Int_t Z, Int_t A, Double_t beta );
		static  Double_t         CalculateMassOverQ( Float_t Brho, Double_t beta );
		static  Double_t         CalculateRealA( Int_t Z,Double_t E, Double_t beta );
		        Double_t         GetBetaFromToF()                    const;
                Float_t          GetBrho()                           const;
                KVVAMOSCodes    &GetCodes();
				using    KVReconstructedNucleus::GetDetector;
				KVDetector      *GetDetector( const Char_t *det_label );
				Float_t          GetEnergy( Int_t idx_det )          const;
				Double_t         GetEnergyBeforeVAMOS()              const;
				Float_t          GetFlightDistance()                 const;
   		const   TVector3        &GetFocalPlaneDirection()            const;
				Double_t         GetGammaFromToF()                   const;
   		const   TVector3        &GetLabDirection()                   const;
                Float_t          GetPath()                           const;
                Float_t          GetPhiF()                           const;
	            Float_t          GetPhiL()                           const;
                Float_t          GetPhiV()                           const;
		virtual Double_t         GetStripFoilEnergyLoss()            const;
        	    Float_t          GetThetaF()                         const;
	            Float_t          GetThetaL()                         const;
                Float_t          GetThetaV()                         const;
				Float_t          GetTimeOfFlight()                   const;
				Float_t          GetToF()                            const;
				Double_t         GetVelocityFromToF()                const;
                Float_t          GetXf()                             const;
	            Float_t          GetYf()                             const;
   		virtual	void             SetECode(UChar_t code_mask);
		virtual void             SetFlightDistanceAndTime(Float_t dist, Float_t time);
		virtual Bool_t           SetFlightDistanceAndTime(Float_t time, KVVAMOSDetector *start, KVVAMOSDetector *stop=NULL);
   		virtual void             SetFPCode(UInt_t code_mask);
 		virtual void             SetIDCode(UShort_t code_mask);
		virtual void             SetStripFoilEnergyLoss( Double_t e);
   		virtual void             SetTCode(UShort_t code_mask);
   		virtual void             SetTCode(const Char_t *parname);
		        void             SetXYf(Float_t x, Float_t y);

		
   		ClassDef(KVVAMOSReconNuc,1)//Nucleus identified by VAMOS spectrometer
};

//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::CalculateEnergy( Int_t Z, Int_t A, Int_t Q, Float_t Brho ){
	// Calculates kinetic energy (E) in MeV.
	// Begin_Latex 
	// E = #sqrt{M^{2}+100 c^{2} B#rho^{2} Q^{2}}- M
	// End_Latex
	// where 
	//   M     : mass deduced from Z and A, in MeV/c^2
	//   c     : speed of light in cm/ns
	//   Brho  : magnetic rigidity in T.m
	//   Q     : charge state
	//
	// If Z is set lower or equal 0 then the mass excess is null to calculate
	// M (i.e. M = A*u)


	Double_t M = 0.;
	if( Z > 0 ){
		static KVNucleus nuc;
		nuc.SetZandA( Z, A );
		M = nuc.GetMass();
	}
	else  M = A*u();
	return TMath::Sqrt( M*M + 100.*TMath::Power(C()*Brho*Q,2))-M;
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::CalculateEnergy( Int_t Z, Int_t A, Double_t beta ){
	// Calculates kinetic energy in MeV.
	// Begin_Latex 
	// E = (#gamma-1)M
	// End_Latex
	// where 
	//   M     : mass deduced from Z and A in, MeV/c^2
	//   gamma : Lorentz factor calculated from beta (velocity/c)
	//
	// If Z is set lower or equal 0 then the mass excess is null to calculate
	// M (i.e. M = A*u)

	Double_t M = 0.;
	if( Z > 0 ){
		static KVNucleus nuc;
		nuc.SetZandA( Z, A );
		M = nuc.GetMass();
	}
	else  M = A*u();
	Double_t gamma = 1.0/TMath::Sqrt( 1 - beta*beta );
return (gamma-1)*M ;
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::CalculateRealA( Int_t Z, Double_t E, Double_t beta ){
	// Calculates the real value of the mass number deduced from the 
	// energy E and and beta (i.e. velocity/c).
	// Begin_Latex 
	// A = #frac{E}{(#gamma-1)u}
	// End_Latex
	// where 
	//   u     : atomic mass unit in MeV/c^2
	//   gamma : Lorentz factor calculated from beta 

	Double_t gamma = 1.0/TMath::Sqrt( 1 - beta*beta );
	if( gamma==1 ) return 0.;
	Double_t realA = E/((gamma-1)*u());
	if ( Z > 0 ){
		static KVNucleus nuc;
		nuc.SetZandA( Z, TMath::Nint(realA) );
		realA -= nuc.GetMassExcess()/u();
	}
	return realA;
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::CalculateMassOverQ( Float_t Brho, Double_t beta ){
	// Calculates the ratio between the mass (MeV/c^2) and the charge state Q
	// calculated from the magnetic rigitidy Brho and the relativistic beta
	// (i.e. velocity/c).
	// The returned value is real.
	// Begin_Latex 
	// #frac{M}{Q} = 10 C #frac{B_{#rho}}{ #gamma #beta}
	// End_Latex
	// where
	//   Brho  : magnetic rigidity in T.m
	//   u     : atomic mass unit in MeV/c^2
	//   C     : speed of light in vacuum in cm/ns 
	//   gamma : Lorentz factor calculated from beta
	
	Double_t gamma = 1.0/TMath::Sqrt( 1 - beta*beta );
	Double_t tmp = beta*gamma;
	if( tmp == 0 ) return 0;
	return Brho*C()*10./tmp;
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::GetBetaFromToF() const{
	return GetVelocityFromToF()/C();
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetBrho() const{
	return fRT.Brho;
}
//____________________________________________________________________________________________//

inline KVVAMOSCodes &KVVAMOSReconNuc::GetCodes(){
	return fCodes;
}
//____________________________________________________________________________________________//

inline KVDetector *KVVAMOSReconNuc::GetDetector( const Char_t *det_label ){
	// Returns the detector in the list of detectors (fDetList)
	// through which particle passed, from its label ("CHI","SI","SED1","SED2",...).

	return GetDetector( GetDetectorIndex( det_label ) );
}
//____________________________________________________________________________________________//

inline Float_t  KVVAMOSReconNuc::GetEnergy( Int_t idx_det ) const{
	// Returns the calculated contribution of each detector to the 
	// nucleus' energy from their index in fDetList. GetEnergy(0) returns
	// the contribution of the stopping detector.
	return ( fDetE && idx_det<GetDetectorList()->GetEntries() ? fDetE[idx_det] : -1 );
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::GetEnergyBeforeVAMOS() const{
	// Kinetic energy of the nucleus prior to entering VAMOS
	return KVReconstructedNucleus::GetEnergy() - GetStripFoilEnergyLoss() - GetTargetEnergyLoss();
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetFlightDistance() const{
	return fFlightDist;
}
//____________________________________________________________________________________________//

inline const TVector3 &KVVAMOSReconNuc::GetFocalPlaneDirection() const{
	// Returns the trajectory normalized direction in the focal-plane frame
	// of reference
	return fRT.dirFP;
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::GetGammaFromToF() const{
	Double_t b = GetBetaFromToF();
	return 1.0/TMath::Sqrt( 1 - b*b );
}
//____________________________________________________________________________________________//

inline const TVector3 &KVVAMOSReconNuc::GetLabDirection() const{
	// Returns the trajectory normalized direction at the target point in the
	// laboratory frame of reference.
	return fRT.dirLab;
}
//____________________________________________________________________________________________//

inline KVVAMOSReconGeoNavigator *KVVAMOSReconNuc::GetNavigator(){
	return gVamos->GetReconNavigator();
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetPath() const{
	return fRT.path;
	// This line is the correction proposed by M. Camano if the reconstruction
	// returns the projection of the real path on the horizontal plane
//	return fRT.path/TMath::Cos( GetPhiV()*TMath::DetToRad() );
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetPhiF() const{
	return fRT.GetPhiF();
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetPhiL() const{
	return fRT.GetPhiL();
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetPhiV() const{
	return fRT.GetPhiV();
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::GetStripFoilEnergyLoss() const{
	// Returns calculated energy loss in stripping foil of reconstructed nucleus ( in MeV )
	return fStripFoilEloss;
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetThetaF() const{
	return fRT.GetThetaF();
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetThetaL() const{
	return fRT.GetThetaL();
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetThetaV() const{
	return fRT.GetThetaV();
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetTimeOfFlight() const{
	return fToF;
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetToF() const{
	return GetTimeOfFlight();
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::GetVelocityFromToF() const{
return (fToF ? fFlightDist/fToF : 0); 
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetXf() const{
	return fRT.pointFP[0];
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetYf() const{
	return fRT.pointFP[1];
}
//____________________________________________________________________________________________//

inline void KVVAMOSReconNuc::SetECode(UChar_t code_mask)
{
   	//Sets code for energy calibration
   	GetCodes().SetECode(code_mask);
}
//____________________________________________________________________________________________//
		
inline void KVVAMOSReconNuc::SetFlightDistanceAndTime(Float_t dist, Float_t time){
	//Sets flight distance in cm and the time of flight in ns;
	//This will change the TCode ( kTCode0 )
	fFlightDist = dist;
	fToF = time;
	SetTCode( kTCode0 );
}
//____________________________________________________________________________________________//
		
inline Bool_t KVVAMOSReconNuc::SetFlightDistanceAndTime(Float_t time, KVVAMOSDetector *start, KVVAMOSDetector *stop){
	//Sets flight distance from the start and the stop detector, and sets the time 
	//of flight in ns;
	//This will change the TCode ( kTCode0 ) and return kTRUE if the distance
	//can be calculated by the reconstructed trajectory between start and stop
	//detectors
	fToF = time;
	SetTCode( kTCode0 );
	return SetFlightDistance( start, stop );
}
//____________________________________________________________________________________________//

inline void KVVAMOSReconNuc::SetFPCode(UInt_t code_mask)
{
   	//Sets code for energy calibration
   	GetCodes().SetFPCode(code_mask);
}
//____________________________________________________________________________________________//

inline void KVVAMOSReconNuc::SetIDCode(UShort_t code_mask)
{
   	//Sets code for identification
   	GetCodes().SetIDCode(code_mask);
}
//____________________________________________________________________________________________//

inline void KVVAMOSReconNuc::SetStripFoilEnergyLoss( Double_t e ){
	// Set energy loss in the stripping foil ( in MeV ) of reconstructed nucleus
	fStripFoilEloss = e;
}
//____________________________________________________________________________________________//

inline void KVVAMOSReconNuc::SetTCode(UShort_t code_mask)
{
   	//Sets code for energy calibration
   	GetCodes().SetTCode(code_mask);
}
//____________________________________________________________________________________________//

inline void KVVAMOSReconNuc::SetTCode(const Char_t *parname){
   	//Sets code for energy calibration from the name of the acquisition 
	//parameter used for the time of flight of the nucleus
   	GetCodes().SetTCode(parname);
}
//____________________________________________________________________________________________//

inline void KVVAMOSReconNuc::SetXYf(Float_t x, Float_t y){
	//Sets the coordinates of the point of intersection (Xf, Yf) of the trajectory
	//and the focal plane in the FP-frame ( in cm ). 
	fRT.pointFP[0] = x;
	fRT.pointFP[1] = y;
}

#endif
