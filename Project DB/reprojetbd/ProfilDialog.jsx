import React, { useState, useEffect } from 'react';
import { Dialog } from 'primereact/dialog';
import { InputText } from 'primereact/inputtext';
import { Button } from 'primereact/button';
import { createProfil, updateProfil } from '../services/ProfilService';

const ProfilDialog = ({ visible, onHide, profil, refresh }) => {
    const [formData, setFormData] = useState({
        libelle: ''
    });

    useEffect(() => {
        if (profil) {
            setFormData({
                id: profil.id,
                libelle: profil.libelle,
            });
        } else {
            setFormData({
                libelle: ''
            });
        }
    }, [profil]);

    const onSubmit = () => {
        console.log("sending profil:", formData);
        if (profil && profil.id) {
            updateProfil(profil.id, formData)
                .then(() => {
                    refresh();
                    onHide();
                })
                .catch(error => console.error(error));
        } else {
            createProfil(formData)
                .then(() => {
                    refresh();
                    onHide();
                })
                .catch(error => console.error(error));
        }
    };

    const footer = (
        <>
            <Button label="Annuler" icon="pi pi-times" onClick={onHide} className="p-button-secondary" />
            <Button label="Enregistrer" icon="pi pi-check" onClick={onSubmit} autoFocus />
        </>
    );

    return (
        <Dialog header="Détails Profil" visible={visible} style={{ width: '30vw' }} footer={footer} onHide={onHide}>
            {/* <div className="p-field">
                <label htmlFor="id">Id</label>
                <InputText id="id" value={formData.id} onChange={(e) => setFormData({ ...formData, id: e.target.value })} />
            </div> */}
            <div className="p-field">
                <label htmlFor="libelle">Libelle</label>
                <InputText id="libelle" value={formData.libelle} onChange={(e) => setFormData({ ...formData, libelle: e.target.value })} />
            </div>
        </Dialog>
    );
};

export default ProfilDialog;