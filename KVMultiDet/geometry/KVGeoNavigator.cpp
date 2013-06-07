//Created by KVClassFactory on Mon Apr 22 14:53:13 2013
//Author: John Frankland,,,

#include "KVGeoNavigator.h"
#include "KVEvent.h"
#include <TGeoManager.h>
#include <TGeoMatrix.h>
#include "KVGeoStrucElement.h"

ClassImp(KVGeoNavigator)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVGeoNavigator</h2>
<h4>Propagate particles of an event through a TGeo geometry</h4>
<!-- */
// --> END_HTML
// This is a base class for propagation of charged particles (KVNucleus) in events (KVEvent)
// through any TGeoManager ROOT geometry. Classes derived from this one can
// override the method
//     ParticleEntersNewVolume(KVNucleus*)
// in order to do something useful every time that a particle of the event
// enters a new volume (absorber, detector, etc.) of the geometry.
//
// Then to use your derived class do something like:
//
//  MyGeoNavigator nav( gGeoManager );
//  while( nevents-- ) {
//       nav.PropagateEvent( event );
//  }
//
// RULES FOR CREATION OF GEOMETRY
// In order for KaliVeda to "understand" your geometry, a few simple rules need
// to be followed.
//
// GEOMETRY STRUCTURE ELEMENTS
// These are used to group detectors and/or other structures.
// They are recognized by their node names, which should be of the form:
//
//     STRUCT_[type]_[number]
//
// STRUCTURE & DETECTOR NAME FORMATTING
//    -- STRUCTURES --
// The default names for structures are taken from the node name by stripping off
// the "STRUCT_" prefix. It is assumed that the remaining string is of the form
//    "[structure type]_[structure number]"
// This is the name that will be used by default for the structure.
// However, this format can be change by calling method
//    SetStructureNameFormat("[structure type]", "[format]")
// where format can contain any of the following tokens:
//    $type$         - will be replaced by the structure type name
//    $type%[fmt]$   - will be replaced by the structure type name using given format
//    $number$       - will be replaced by the structure number
//    $number%[fmt]$ - will be replaced by the structure number using given format
//
// Example: to change the name of the block in the previous example to "B-02",
//   SetStructureNameFormat("BLOCK", "$type%.1s$-$number%02d$")
//
//    -- DETECTORS --
// The default base names for detectors are taken from the node name by stripping off
// the "DET_" prefix. In order to ensure that all detectors have unique names,
// by default we prefix the names of the parent structures to the basename in
// order to generate the full name of the detector:
//    [struc1-name]_[struc2-name]_..._[detector-basename]
// However, this format can be changed by calling method
//    SetDetectorNameFormat("[format]")
// where format can contain any of the following tokens:
//    $det:name$             - will be replaced by the detector basename
//    $struc:[type]:name$    - will be replaced by the name of the parent structure of given type
//    $struc:[type]:type$    - will be replaced by the type of the parent structure of given type
//    $struc:[type]:number$  - will be replaced by the number of the parent structure of given type
// plus additional formatting information as for SetStructureNameFormat (see above).
//
// Example: to change the name of the "SI1-T1" detector in the previous example to
//  "SI1-T1-Q1-B2":
//   SetDetectorNameFormat("$det:name$-Q$struc:QUARTET:number$-B$struc:BLOCK:number$")
// Or if you also change the format of the structure names:
//   SetStructureNameFormat("BLOCK", "$type%.1s$$number$")
//   SetStructureNameFormat("QUARTET", "$type%.1s$$number$")
//   SetDetectorNameFormat("$det:name$-$struc:QUARTET:name$-$struc:BLOCK:name$")
////////////////////////////////////////////////////////////////////////////////

void KVGeoNavigator::FormatStructureName(const Char_t *type, Int_t number, KVString &name)
{
    // If a format for naming structures of given type has been defined by a call
    // to SetStructureNameFormat(const Char_t *, const Char_t *), we use it to
    // format the name in the TString.
    // If no format was given, we use by default "[type]_[number]"

    name="";
    if(fStrucNameFmt.HasParameter(type)){
        KVString fmt = fStrucNameFmt.GetStringValue(type);
        fmt.Begin("$");
        while(!fmt.End()){
            KVString bit = fmt.Next();
            if(bit.BeginsWith("type")){
                Ssiz_t ind = bit.Index("%");
                if(ind>-1) {
                    bit.Remove(0,ind);
                    name+=Form(bit.Data(),type);
                }
                else
                    name+=type;
            }
            else if(bit.BeginsWith("number")){
                Ssiz_t ind = bit.Index("%");
                if(ind>-1) {
                    bit.Remove(0,ind);
                    name+=Form(bit.Data(),number);
                }
                else
                    name+=number;
            }
            else
                name+=bit;
        }
    }
    else
        name.Form("%s_%d",type,number);
}

void KVGeoNavigator::FormatDetectorName(const Char_t *basename, KVString &name)
{
    // If a format for naming detectors has been defined by a call
    // to SetDetectorNameFormat(const Char_t *), we use it to
    // format the name in the TString.
    // If no format was given we prefix the names of the parent structures
    // to the basename in order to generate the full name of the detector:
    //    [struc1-name]_[struc2-name]_..._[detector-basename]

    name = "";
    if(!fCurStrucNumber){
        // no parent structures
        name = basename;
    }
    else
    {
        if(fDetNameFmt==""){
            for(register int i=0;i<fCurStrucNumber;i++){
                KVGeoStrucElement* el = (KVGeoStrucElement*)fCurrentStructures[i];
                name+=Form("%s_",el->GetName());
            }
            name+=basename;
        }
        else
        {
            //    $det:name$             - will be replaced by the detector basename
            //    $struc:[type]:name$    - will be replaced by the name of the parent structure of given type
            //    $struc:[type]:type$    - will be replaced by the type of the parent structure of given type
            //    $struc:[type]:number$  - will be replaced by the number of the parent structure of given type
            fDetNameFmt.Begin("$");
            while(!fDetNameFmt.End()){
                KVString bit = fDetNameFmt.Next();
                if(bit.Contains(":")){
                    bit.Begin(":");
                    KVString itbit = bit.Next();
                    if(itbit=="det"){
                        itbit=bit.Next();
                        if(itbit=="name") name+=basename;
                    }
                    else if(itbit=="struc"){
                        KVString struc_typ=bit.Next();
                        KVGeoStrucElement* el=0;
                        for(register int i=0;i<fCurStrucNumber;i++){
                            el = (KVGeoStrucElement*)fCurrentStructures[i];
                            if(el->IsType(struc_typ)) break;
                        }
                        if(el){
                            itbit=bit.Next();
                            if(itbit.BeginsWith("name")){
                                Ssiz_t ind = bit.Index("%");
                                if(ind>-1) {
                                    itbit.Remove(0,ind);
                                    name+=Form(itbit.Data(),el->GetName());
                                }
                                else
                                    name+=el->GetName();
                            }
                            else if(itbit.BeginsWith("type")){
                                Ssiz_t ind = itbit.Index("%");
                                if(ind>-1) {
                                    itbit.Remove(0,ind);
                                    name+=Form(itbit.Data(),el->GetType());
                                }
                                else
                                    name+=el->GetType();
                            }
                            else if(itbit.BeginsWith("number")){
                                Ssiz_t ind = itbit.Index("%");
                                if(ind>-1) {
                                    itbit.Remove(0,ind);
                                    name+=Form(itbit.Data(),el->GetNumber());
                                }
                                else
                                    name+=el->GetNumber();
                            }
                        }
                    }
                }
                else
                    name+=bit;
            }
        }
    }
}

KVGeoNavigator::KVGeoNavigator(TGeoManager *g)
    : fCurrentStructures("KVGeoStrucElement",50)
{
    // Constructor. Call with pointer to geometry.
    fGeometry = g;
}

KVGeoNavigator::~KVGeoNavigator()
{
    // Destructor
    fCurrentStructures.Delete();
}

void KVGeoNavigator::SetStructureNameFormat(const Char_t *type, const Char_t *fmt)
{
    // The default names for structures are taken from the node name by stripping off
    // the "STRUCT_" prefix. It is assumed that the remaining string is of the form
    //    "[structure type]_[structure number]"
    // This is the name that will be used by default for the structure.
    // However, this format can be change by calling method
    //    SetStructureNameFormat("[structure type]", "[format]")
    // where format can contain any of the following tokens:
    //    $type$         - will be replaced by the structure type name
    //    $type%[fmt]$   - will be replaced by the structure type name using given format
    //    $number$       - will be replaced by the structure number
    //    $number%[fmt]$ - will be replaced by the structure number using given format
    fStrucNameFmt.SetValue(type,fmt);
}

void KVGeoNavigator::PropagateEvent(KVEvent *TheEvent, TVector3 *TheOrigin)
{
    // Propagate a set of particles through the geometry
    // By default, propagates particles from (0,0,0) (world coordinates),
    // unless a different origin is given.
    KVNucleus* part;
    while( (part = TheEvent->GetNextParticle()) ){
        PropagateParticle(part, TheOrigin);
    }
}

void KVGeoNavigator::ParticleEntersNewVolume(KVNucleus*)
{
    // User-overridable method, redefine in child classes
    // This method is called every time that a propagated particle enters a new volume
    // in the geometry.
    // The user then has access to the following informations:
    //
    // - the TGeoVolume the particle is now entering (GetCurrentVolume());
    // - the node in the geometry this volume occupies (GetCurrentNode());
    // - the distance the particle will have to travel in this volume before
    //    leaving it (GetStepSize());
    // - the position of the particle on the boundary as it enters this volume
    //    (GetEntryPoint()).
    // - the position of the particle on the boundary as it leaves this volume
    //    (GetExitPoint()).
    //
    // If required, propagation of the particle can be stopped at any time by calling
    //      SetStopPropagation()

    AbstractMethod("ParticleEntersNewVolume");
}

TGeoHMatrix *KVGeoNavigator::GetCurrentMatrix() const
{
    // Returns pointer to current global transformation matrix
    return fCurrentMatrix;
}

TGeoVolume *KVGeoNavigator::GetCurrentDetectorNameAndVolume(KVString &detector_name, Bool_t& multilayer)
{
    // Returns the name of the current detector (if we are inside a detector)
    // and whether it is a multilayer or simple detector.
    // Returns 0x0 if we are not inside a detector volume.
    //
    // N.B. the returned volume corresponds to the *whole* detector (even if it has several layers).
    // For a multilayer detector, GetCurrentVolume() returns the volume for the current layer.
    //
    // See ExtractDetectorNameFromPath(KVString&) for details on detector name formatting.

    multilayer=kFALSE;
    fCurrentDetectorNode=0;
    TString volNom = GetCurrentVolume()->GetName();
    TGeoVolume* detector_volume=0;
    if(volNom.BeginsWith("DET_")){
        // simple detector
        fCurrentDetectorNode = GetCurrentNode();
        detector_volume = GetCurrentVolume();
    }
    else
    {
        // have we hit 1 layer of a multilayer detector?
        TGeoVolume* mother_vol = GetCurrentNode()->GetMotherVolume();
        if(mother_vol) {
            TString mom = mother_vol->GetName();
            if(mom.BeginsWith("DET_")){
                // it *is* a multilayer detector (youpi! :-)
                if(fMotherNode) {// this is the node corresponding to the whole detector,
                    fCurrentDetectorNode = fMotherNode;
                    detector_volume = mother_vol;
                    multilayer=kTRUE;
                }
            }
        }
    }
    if(detector_volume) ExtractDetectorNameFromPath(detector_name);
    return detector_volume;
}

TGeoNode *KVGeoNavigator::GetCurrentDetectorNode() const
{
    // Returns the node corresponding to the current detector volume
    // N.B. the returned node corresponds to the *whole* detector (even if it has several layers).
    return fCurrentDetectorNode;
}

void KVGeoNavigator::ExtractDetectorNameFromPath(KVString &detname)
{
    // We analyse the current path in order to construct the full (unique) name
    // of the detector, i.e. if the current path is
    //
    // /TOP_1/STRUCT_BLOCK_2/CHIO_WALL_1/DET_CHIO_2/WINDOW_1
    //
    // then the default name of the detector will be "BLOCK_2_CHIO_2"
    // (see below to override this)
    //
    // This method also fills the fCurrentStructures array with elements
    // deduced from the path, e.g. if the path is
    //
    // /TOP_1/STRUCT_BLOCK_2/STRUCT_QUARTET_1/DET_SI1-T1
    //
    // then by default
    //  fCurrentStructures[0] = KVGeoStrucElement(name = "BLOCK_2", type = "BLOCK", number = 2)
    //  fCurrentStructures[1] = KVGeoStrucElement(name = "QUARTET_1", type = "QUARTET", number = 1)
    //
    // and the default name of the detector will be "BLOCK_2_QUARTET_1_SI1-T1"
    //
    // STRUCTURE & DETECTOR NAME FORMATTING
    // ====================================
    //    -- STRUCTURES --
    // The default names for structures are taken from the node name by stripping off
    // the "STRUCT_" prefix. It is assumed that the remaining string is of the form
    //    "[structure type]_[structure number]"
    // This is the name that will be used by default for the structure.
    // However, this format can be change by calling method
    //    SetStructureNameFormat("[structure type]", "[format]")
    // where format can contain any of the following tokens:
    //    $type$         - will be replaced by the structure type name
    //    $type%[fmt]$   - will be replaced by the structure type name using given format
    //    $number$       - will be replaced by the structure number
    //    $number%[fmt]$ - will be replaced by the structure number using given format
    //
    // Example: to change the name of the block in the previous example to "B-02",
    //   SetStructureNameFormat("BLOCK", "$type%.1s$-$number%02d$")
    //
    //    -- DETECTORS --
    // The default base names for detectors are taken from the node name by stripping off
    // the "DET_" prefix. In order to ensure that all detectors have unique names,
    // by default we prefix the names of the parent structures to the basename in
    // order to generate the full name of the detector:
    //    [struc1-name]_[struc2-name]_..._[detector-basename]
    // However, this format can be changed by calling method
    //    SetDetectorNameFormat("[format]")
    // where format can contain any of the following tokens:
    //    $det:name$             - will be replaced by the detector basename
    //    $struc:[type]:name$    - will be replaced by the name of the parent structure of given type
    //    $struc:[type]:type$    - will be replaced by the type of the parent structure of given type
    //    $struc:[type]:number$  - will be replaced by the number of the parent structure of given type
    // plus additional formatting information as for SetStructureNameFormat (see above).
    //
    // Example: to change the name of the "SI1-T1" detector in the previous example to
    //  "SI1-T1-Q1-B2":
    //   SetDetectorNameFormat("$det:name$-Q$struc:QUARTET:number$-B$struc:BLOCK:number$")
    // Or if you also change the format of the structure names:
    //   SetStructureNameFormat("BLOCK", "$type%.1s$$number$")
    //   SetStructureNameFormat("QUARTET", "$type%.1s$$number$")
    //   SetDetectorNameFormat("$det:name$-$struc:QUARTET:name$-$struc:BLOCK:name$")


    KVString path = GetCurrentPath();
    path.Begin("/");
    detname="";
    fCurrentStructures.Clear("C");
    fCurStrucNumber=0;
    while(!path.End()){
        KVString elem = path.Next();
        if(elem.BeginsWith("STRUCT_")){
            // structure element. strip off "STRUCT_" and extract type and number of structure.
            KVString struc_name(elem(7,elem.Length()-7));
            KVGeoStrucElement* gel = (KVGeoStrucElement*)fCurrentStructures.ConstructedAt(fCurStrucNumber++);
            struc_name.Begin("_");
            KVString type = struc_name.Next();
            Int_t number = struc_name.Next().Atoi();
            KVString name;
            FormatStructureName(type,number,name);
            gel->SetNameTitle(name, type);
            gel->SetNumber(number);
        }
        else if(elem.BeginsWith("DET_")){
            // detector name. strip off "DET_" and use rest as basename
            KVString basename(elem(4,elem.Length()-4));
            FormatDetectorName(basename,detname);
        }
    }
}

void KVGeoNavigator::PropagateParticle(KVNucleus*part, TVector3 *TheOrigin)
{
    // Propagate a particle through the geometry in the direction of its momentum,
    // until we reach the boundary of the geometry, or until fStopPropagation is set to kFALSE.
    // Propagation will also stop if we encounter a volume whose name begins with "DEADZONE"

    // Define point of origin of particles
    if(TheOrigin) fGeometry->SetCurrentPoint(TheOrigin->X(),TheOrigin->Y(),TheOrigin->Z());
    else fGeometry->SetCurrentPoint(0., 0., 0.);

    // unit vector in direction of particle's momentum
    TVector3 v = part->GetMomentum().Unit();
    // use particle's momentum direction
    fGeometry->SetCurrentDirection(v.x(), v.y(), v.z());
    fGeometry->FindNode();

    fCurrentVolume = fGeometry->GetCurrentVolume();
    fCurrentNode = fGeometry->GetCurrentNode();
    fMotherNode = fGeometry->GetMother();
    fCurrentMatrix = fGeometry->GetCurrentMatrix();
    fCurrentPath = fGeometry->GetPath();
    // move along trajectory until we hit a new volume
    fGeometry->FindNextBoundaryAndStep();
    fStepSize = fGeometry->GetStep();
    TGeoVolume* newVol = fGeometry->GetCurrentVolume();
    TGeoNode* newNod = fGeometry->GetCurrentNode();
    TGeoNode* newMom = fGeometry->GetMother();
    TGeoHMatrix* newMatx = fGeometry->GetCurrentMatrix();
    TString newPath = fGeometry->GetPath();

    Double_t XX,YY,ZZ;
    XX=YY=ZZ=0.;

    // reset user flag for stopping propagation of particle
    SetStopPropagation(kFALSE);

    // track particle until we leave the geometry or until fStopPropagation
    // becomes kTRUE
    while (!fGeometry->IsOutside()) {

        const Double_t* posi = fGeometry->GetCurrentPoint();
        fEntryPoint.SetXYZ(XX,YY,ZZ);
        XX=posi[0];YY=posi[1];ZZ=posi[2];
        fExitPoint.SetXYZ(XX,YY,ZZ);

        TString vn = GetCurrentVolume()->GetName();
        if(vn.BeginsWith("DEADZONE")) break;

        ParticleEntersNewVolume(part);

        if(StopPropagation()) break;

        fCurrentVolume = newVol;
        fCurrentNode = newNod;
        fMotherNode = newMom;
        fCurrentMatrix = newMatx;
        fCurrentPath = newPath;

        // move on to next volume crossed by trajectory
        fGeometry->FindNextBoundaryAndStep();
        fStepSize = fGeometry->GetStep();
        newVol = fGeometry->GetCurrentVolume();
        newNod = fGeometry->GetCurrentNode();
        newMom = fGeometry->GetMother();
        newMatx = fGeometry->GetCurrentMatrix();
        newPath = fGeometry->GetPath();
    }
}

