import React, { useState, useEffect } from 'react';
import { Dialog } from 'primereact/dialog';
import { InputText } from 'primereact/inputtext';
import { Button } from 'primereact/button';
import { createStructure, updateStructure } from '../services/StructureService';

const StructureDialog = ({ visible, onHide, structure, refresh }) => {
    const [formData, setFormData] = useState({
        id: '',
        libelle: ''
    });

    useEffect(() => {
        if (structure) {
            setFormData({
                id: structure.id,
                libelle: structure.libelle,
            });
        } else {
            setFormData({
                id: '',
                libelle: ''
            });
        }
    }, [structure]);

    const onSubmit = () => {
        console.log("sending structure:", formData);
        if (structure && structure.id) {
            updateStructure(structure.id, formData)
                .then(() => {
                    refresh();
                    onHide();
                })
                .catch(error => console.error(error));
        } else {
            createStructure(formData)
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
        <Dialog header="Détails Structure" visible={visible} style={{ width: '30vw' }} footer={footer} onHide={onHide}>
            <div className="p-field">
                <label htmlFor="id">Id</label>
                <InputText id="id" value={formData.id} onChange={(e) => setFormData({ ...formData, id: e.target.value })} />
            </div>
            <div className="p-field">
                <label htmlFor="libelle">Libelle</label>
                <InputText id="libelle" value={formData.libelle} onChange={(e) => setFormData({ ...formData, libelle: e.target.value })} />
            </div>
        </Dialog>
    );
};

export default StructureDialog;