import React, { useState, useEffect } from 'react';
import { DataTable } from 'primereact/datatable';
import { Column } from 'primereact/column';
import { Toolbar } from 'primereact/toolbar';
import { Button } from 'primereact/button';
import UserDialog from './UserDialog';
import ConfirmDialogBox from './ConfirmDialogBox';
import { getUsers,deleteUser } from '../services/UserService';

const UserTable = () => {
    const [Users, setUsers] = useState([]);
    const [selectedUser, setSelectedUser] = useState(null);
    const [selectedUsers, setSelectedUsers] = useState([]);
    const [dialogVisible, setDialogVisible] = useState(false);
    const [confirmVisible, setConfirmVisible] = useState(false);

    useEffect(() => {
        fetchUsers();
    }, []);

    const fetchUsers = () => {
        getUsers().then(response => {
            setUsers(response.data);
        }).catch(error => console.error(error));
    };

    const openNew = () => {
        setSelectedUser(null);
        setDialogVisible(true);
    };

    const editUser = (user) => {
        setSelectedUser(user);
        setDialogVisible(true);
    };

    const deleteUserById = (user) => {
        deleteUser(user.id)
            .then(() => {
                fetchUsers();
                setConfirmVisible(false);
            })
            .catch(error => console.error(error));
    };

    const leftToolbarTemplate = () => (
        <>
            <Button label="Nouveau" icon="pi pi-plus" className="p-button-success" onClick={openNew} />
            <Button label="Supprimer" icon="pi pi-trash" className="p-button-danger"
                disabled={!selectedUsers || !selectedUsers.length}
                onClick={() => setConfirmVisible(true)} />
        </>
    );

    const actionBodyTemplate = (rowData) => (
        <>
<Button
  style={{ width: '50px', alignItems: 'center' , marginLeft:'8%'}}
  icon="pi pi-pencil"
  className="p-button-rounded p-button-success mr-2"
  onClick={() => editUser(rowData)}
/>            <Button style={{width:'50px'}}  icon="pi pi-trash" className="p-button-rounded p-button-warning" onClick={() => deleteUserById(rowData)} />
        </>
    );

    return (
        <div className="card">
            <Toolbar className="mb-4" left={leftToolbarTemplate} />
            <DataTable value={Users} selection={selectedUsers} onSelectionChange={e => setSelectedUsers(e.value)}
                dataKey="id" paginator rows={10} header="Liste des Fonctions">
                <Column selectionMode="multiple" headerStyle={{ width: '3em' }}></Column>
                <Column field="id" header="Id" sortable></Column>
                <Column field="login" header="login" sortable></Column>
                <Column field="password" header="password" sortable></Column>


                <Column field="idRole.id" header="role_ID" sortable></Column>
                <Column field="idRole.nom" header="role" sortable></Column>
                <Column body={actionBodyTemplate}></Column>
            </DataTable>

            {/* Dialogue pour ajout/édition */}
            <UserDialog visible={dialogVisible} 
                               onHide={() => setDialogVisible(false)} 
                               user={selectedUser} 
                               refresh={fetchUsers} />

            {/* Dialogue de confirmation (delete multiple ou confirmation simple) */}
            <ConfirmDialogBox visible={confirmVisible} 
                               onHide={() => setConfirmVisible(false)}
                               onConfirm={() => {
                                   selectedUsers.forEach(p => deleteUserById(p));
                                   setSelectedUsers([]);
                               }} 
                               message="Confirmez-vous la suppression des user sélectionnés ?" />
        </div> 
    );
};

export default UserTable;