import React, { useState, useEffect } from 'react';
import { DataTable } from 'primereact/datatable';
import { Column } from 'primereact/column';
import { Toolbar } from 'primereact/toolbar';
import { Button } from 'primereact/button';
import ProfilDialog from './ProfilDialog';
import ConfirmDialogBox from './ConfirmDialogBox';
import { getProfils,deleteProfil } from '../services/ProfilService';

const ProfilTable = () => {
    const [Profils, setProfils] = useState([]);
    const [selectedProfil, setSelectedProfil] = useState(null);
    const [selectedProfils, setSelectedProfils] = useState([]);
    const [dialogVisible, setDialogVisible] = useState(false);
    const [confirmVisible, setConfirmVisible] = useState(false);

    useEffect(() => {
        fetchProfils();
    }, []);

    const fetchProfils = () => {
        getProfils().then(response => {
            setProfils(response.data);
        }).catch(error => console.error(error));
    };

    const openNew = () => {
        setSelectedProfil(null);
        setDialogVisible(true);
    };

    const editProfil = (profil) => {
        setSelectedProfil(profil);
        setDialogVisible(true);
    };

    const deleteProfilById = (profil) => {
        deleteProfil(profil.id)
            .then(() => {
                fetchProfils();
                setConfirmVisible(false);
            })
            .catch(error => console.error(error));
    };

    const leftToolbarTemplate = () => (
        <>
            <Button label="Nouveau" icon="pi pi-plus" className="p-button-success" onClick={openNew} />
            <Button label="Supprimer" icon="pi pi-trash" className="p-button-danger"
                disabled={!selectedProfils || !selectedProfils.length}
                onClick={() => setConfirmVisible(true)} />
        </>
    );

    const actionBodyTemplate = (rowData) => (
        <>
<Button
  style={{ width: '50px', alignItems: 'center' , marginLeft:'8%'}}
  icon="pi pi-pencil"
  className="p-button-rounded p-button-success mr-2"
  onClick={() => editProfil(rowData)}
/>            <Button style={{width:'50px'}}  icon="pi pi-trash" className="p-button-rounded p-button-warning" onClick={() => deleteProfilById(rowData)} />
        </>
    );

    return (
        <div className="card">
            <Toolbar className="mb-4" left={leftToolbarTemplate} />
            <DataTable value={Profils} selection={selectedProfils} onSelectionChange={e => setSelectedProfils(e.value)}
                dataKey="id" paginator rows={10} header="Liste des Profilss">
                <Column selectionMode="multiple" headerStyle={{ width: '3em' }}></Column>
                <Column field="id" header="Id" sortable></Column>
                <Column field="libelle" header="Libelle" sortable></Column>
                <Column body={actionBodyTemplate}></Column>
            </DataTable>

            {/* Dialogue pour ajout/édition */}
            <ProfilDialog visible={dialogVisible} 
                               onHide={() => setDialogVisible(false)} 
                               profil={selectedProfil} 
                               refresh={fetchProfils} />

            {/* Dialogue de confirmation (delete multiple ou confirmation simple) */}
            <ConfirmDialogBox visible={confirmVisible} 
                               onHide={() => setConfirmVisible(false)}
                               onConfirm={() => {
                                   selectedProfils.forEach(p => deleteProfilById(p));
                                   setSelectedProfils([]);
                               }} 
                               message="Confirmez-vous la suppression des profiles sélectionnés ?" />
        </div>
    );
};

export default ProfilTable;