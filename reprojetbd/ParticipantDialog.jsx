import React, { useState, useEffect } from 'react';
import { Dialog } from 'primereact/dialog';
import { InputText } from 'primereact/inputtext';
import { Button } from 'primereact/button';
import { Dropdown } from 'primereact/dropdown';
import { Message } from 'primereact/message';
import { createParticipant, updateParticipant } from '../services/ParticipantService';
import { getProfils } from '../services/ProfilService';
import { getStructures } from '../services/StructureService';

const ParticipantDialog = ({ visible, onHide, participant, refresh }) => {
    // État initial simplifié
    const initialFormState = {
        email: '',
        nom: '',
        prenom: '',
        tel: '',
        profilId: null,
        structureId: null
    };

    const [formData, setFormData] = useState(initialFormState);
    const [profils, setProfils] = useState([]);
    const [structures, setStructures] = useState([]);
    const [loading, setLoading] = useState(false);
    const [error, setError] = useState(null);

    // Chargement des données et initialisation
    useEffect(() => {
        if (!visible) return;

        const loadData = async () => {
            try {
                setLoading(true);
                const [profilsResponse, structuresResponse] = await Promise.all([
                    getProfils(),
                    getStructures()
                ]);

                setProfils(profilsResponse.data);
                setStructures(structuresResponse.data);

                if (participant) {
                    setFormData({
                        email: participant.email,
                        nom: participant.nom,
                        prenom: participant.prenom,
                        tel: participant.tel,
                        profilId: participant.idprofil?.id || null,
                        structureId: participant.idstructure?.id || null
                    });
                }
            } catch (err) {
                setError("Erreur de chargement des données");
                console.error("Erreur:", err);
            } finally {
                setLoading(false);
            }
        };

        loadData();

        return () => {
            // Reset à la fermeture
            setFormData(initialFormState);
            setError(null);
        };
    }, [visible, participant]);

    const handleChange = (field, value) => {
        setFormData(prev => ({ ...prev, [field]: value }));
    };

    const handleSubmit = async () => {
        // Validation
        if (!formData.email || !formData.nom || !formData.prenom || !formData.tel) {
            setError("Tous les champs textuels sont obligatoires");
            return;
        }

        if (!formData.profilId || !formData.structureId) {
            setError("Veuillez sélectionner un profil et une structure");
            return;
        }

        try {
            setLoading(true);
            setError(null);

            const payload = {
                email: formData.email,
                nom: formData.nom,
                prenom: formData.prenom,
                tel: formData.tel,
                idprofil: { id: formData.profilId },
                idstructure: { id: formData.structureId }
            };

            console.log("Payload envoyé:", JSON.stringify(payload, null, 2));

            if (participant?.id) {
                await updateParticipant(participant.id, payload);
            } else {
                await createParticipant(payload);
            }

            refresh();
            onHide();
        } catch (err) {
            const errorMsg = err.response?.data?.message || 
                           err.message || 
                           "Erreur lors de l'enregistrement";
            setError(errorMsg);
            console.error("Erreur API:", err);
        } finally {
            setLoading(false);
        }
    };

    return (
        <Dialog
            header={participant ? "Modifier Participant" : "Nouveau Participant"}
            visible={visible}
            style={{ width: '40vw' }}
            onHide={onHide}
            modal
            footer={
                <div>
                    {error && <Message severity="error" text={error} className="mb-3" />}
                    <div className="flex justify-content-end gap-2">
                        <Button
                            label="Annuler"
                            icon="pi pi-times"
                            onClick={onHide}
                            className="p-button-text"
                            disabled={loading}
                        />
                        <Button
                            label="Enregistrer"
                            icon="pi pi-check"
                            onClick={handleSubmit}
                            loading={loading}
                            disabled={loading}
                        />
                    </div>
                </div>
            }
        >
            <div className="p-fluid grid formgrid">
                <div className="field col-12 md:col-6">
                    <label htmlFor="nom">Nom*</label>
                    <InputText
                        id="nom"
                        value={formData.nom}
                        onChange={(e) => handleChange('nom', e.target.value)}
                        disabled={loading}
                        className="w-full"
                    />
                </div>

                <div className="field col-12 md:col-6">
                    <label htmlFor="prenom">Prénom*</label>
                    <InputText
                        id="prenom"
                        value={formData.prenom}
                        onChange={(e) => handleChange('prenom', e.target.value)}
                        disabled={loading}
                        className="w-full"
                    />
                </div>

                <div className="field col-12 md:col-6">
                    <label htmlFor="email">Email*</label>
                    <InputText
                        id="email"
                        value={formData.email}
                        onChange={(e) => handleChange('email', e.target.value)}
                        disabled={loading}
                        type="email"
                        className="w-full"
                    />
                </div>

                <div className="field col-12 md:col-6">
                    <label htmlFor="tel">Téléphone*</label>
                    <InputText
                        id="tel"
                        value={formData.tel}
                        onChange={(e) => handleChange('tel', e.target.value)}
                        disabled={loading}
                        className="w-full"
                    />
                </div>

                <div className="field col-12 md:col-6">
                    <label htmlFor="profil">Profil*</label>
                    <Dropdown
                        value={formData.profilId}
                        options={profils}
                        onChange={(e) => handleChange('profilId', e.value)}
                        optionLabel="libelle"
                        optionValue="id"
                        placeholder="Sélectionner"
                        disabled={loading || profils.length === 0}
                        className={`w-full ${!formData.profilId ? 'p-invalid' : ''}`}
                    />
                </div>

                <div className="field col-12 md:col-6">
                    <label htmlFor="structure">Structure*</label>
                    <Dropdown
                        value={formData.structureId}
                        options={structures}
                        onChange={(e) => handleChange('structureId', e.value)}
                        optionLabel="libelle"
                        optionValue="id"
                        placeholder="Sélectionner"
                        disabled={loading || structures.length === 0}
                        className={`w-full ${!formData.structureId ? 'p-invalid' : ''}`}
                    />
                </div>
            </div>
        </Dialog>
    );
};

export default ParticipantDialog;