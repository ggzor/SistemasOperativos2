import * as React from "react"
import styled from "styled-components"
import {
  DesktopDownload,
  Package,
  Info,
  Check,
  Sync,
  ListUnordered,
  Graph,
  Clock,
  X,
  FileDirectory,
  File,
  IssueReopened,
} from "@primer/octicons-react"
import { boolToVisibility } from "./utils"

import { IconButton } from "./IconButton"
import { IconShell } from "./IconShell"
import { useState } from "react"

import * as Colors from "./Colors"
import { LoadingIndicator } from "./LoadingIndicator"

export const ComputerCard = ({
  name,
  info: { isLocal, ...info },
  fileSystem,
  onEvent,
}) => {
  const canReceive = true
  const [selectedTab, setSelectedTab] = useState("archivos")
  const [selectedFiles, setSelectedFiles] = useState(["abcd", "cdef"])

  const flatFS = flattenFileSystem(fileSystem)

  return (
    <ComputerCardLayout>
      <TopBar visible={isLocal} />
      <InnerContent>
        <Header>
          <h2>{name}</h2>
          {canReceive && (
            <IconButton
              icon={DesktopDownload}
              onClick={() => onEvent && onEvent({ type: "receive", name })}
            >
              Recibir
            </IconButton>
          )}
        </Header>
        <Chip visible={isLocal}>Local</Chip>
        {selectedTab === "archivos" &&
          (flatFS !== null ? (
            <FileSystemLayout>
              {flatFS.map(([depth, file]) => (
                <FileItem
                  depth={depth}
                  selected={selectedFiles.indexOf(file.hash) >= 0}
                  key={file.hash}
                  {...file}
                />
              ))}
            </FileSystemLayout>
          ) : (
            <EmptyFs />
          ))}
        {selectedTab === "informacion" && (
          <ComputerInfo>
            <InfoDetail
              icon={iconForStatus[info.status]}
              iconColor={colorForStatus[info.status]}
            >
              <span
                style={{
                  display: "inline-block",
                  color: colorForStatus[info.status],
                  fontWeight: "normal",
                  marginTop: "2px",
                }}
              >
                {labelForStatus[info.status]}
              </span>
            </InfoDetail>
            <InfoDetail icon={Sync} iconColor={Colors.primary}>
              {info.replica !== null ? (
                <>
                  Replicado por <b>{info.replica}</b>
                  <br />
                  <span
                    style={{
                      color: colorForSyncState[info.syncState.state],
                      fontWeight: "normal",
                    }}
                  >
                    {formatSyncState(info.syncState)}
                  </span>
                </>
              ) : (
                <>Replicador aún no asignado</>
              )}
            </InfoDetail>
            <InfoDetail icon={ListUnordered} iconColor={Colors.primary}>
              {info.fileCount !== null ? (
                <>
                  <b>{info.fileCount}</b> archivos
                  <br />
                  <b>{formatByteSize(info.totalFileSize)}</b> de datos en total
                  <br />
                </>
              ) : (
                <>Obteniendo información de los archivos...</>
              )}
            </InfoDetail>
            <InfoDetail icon={Graph} iconColor={Colors.primary}>
              {info.sentFiles !== null ? (
                <>
                  <b>{info.sentFiles}</b> archivos enviado
                  {info.sentFiles !== 1 && "s"}
                  <br />
                  <b>{info.receivedFiles}</b> archivos recibido
                  {info.receivedFiles !== 1 && "s"}
                  <br />
                </>
              ) : (
                <>Calculando métricas de las transferencias...</>
              )}
            </InfoDetail>
          </ComputerInfo>
        )}
      </InnerContent>
      <BottomSeparator />
      <BottomSection>
        <BottomTabButton
          selected={selectedTab === "archivos"}
          onClick={() => setSelectedTab("archivos")}
          text="Archivos"
          icon={Package}
        />
        <BottomTabButton
          selected={selectedTab === "informacion"}
          onClick={() => setSelectedTab("informacion")}
          text="Información"
          icon={Info}
        />
      </BottomSection>
    </ComputerCardLayout>
  )
}

function flattenFileSystem(fs, depth = 0) {
  if (fs === null) {
    return null
  } else if (fs.type === "file") {
    return [[depth, fs]]
  } else
    return [
      [depth, fs],
      ...fs.files.flatMap((f) => flattenFileSystem(f, depth + 1)),
    ]
}

const iconForStatus = {
  online: Check,
  replica: Clock,
  offline: X,
  pending: IssueReopened,
}
const colorForStatus = {
  online: Colors.success,
  replica: Colors.success,
  offline: Colors.fail,
  pending: Colors.warning,
}
const labelForStatus = {
  online: "En línea",
  replica: "Sólo réplica disponible",
  offline: "No disponible",
  pending: "Pendiente",
}
const colorForSyncState = { synced: Colors.success, syncing: Colors.warning }

function formatSyncState(state) {
  if (state.state === "syncing") {
    return `Aplicando cambios ${state.progress}/${state.total}`
  } else if (state.state === "synced") {
    return "Sincronizado"
  }
}

function formatByteSize(size) {
  return `${size / 1e6}Mb`
}

const ComputerCardLayout = styled.article`
  width: 280px;
  height: 380px;

  display: grid;
  grid-template-rows: auto 1fr auto auto;

  background-color: white;
  box-shadow: 1px 10px 20px rgba(0, 0, 0, 0.1), 0px 5px 8px rgba(0, 0, 0, 0.15);
  border-radius: 10px;
`

const InnerContent = styled.section`
  display: flex;
  flex-direction: column;
  overflow: hidden;

  & > *:last-child {
    flex-grow: 1;
  }

  flex-grow: 1;
  padding: 16px;
  margin-top: 0px;
`

const TopBar = styled.div`
  visibility: ${({ visible }) => boolToVisibility(visible)};

  height: 6px;
  background-color: var(--color-success);

  border-top-left-radius: 10px;
  border-top-right-radius: 10px;
`

const Header = styled.section`
  display: flex;
  user-select: none;

  & > *:first-child {
    flex-grow: 1;
  }
`
const Chip = styled.span`
  visibility: ${({ visible }) => boolToVisibility(visible)};

  align-self: flex-start;
  border-radius: 10px;
  padding: 4px 8px;
  text-transform: uppercase;
  background-color: var(--color-success);

  font-family: Roboto;
  font-style: normal;
  font-weight: bold;
  font-size: 12px;
  line-height: 11px;

  letter-spacing: 0.05em;

  color: #ffffff;
  display: inline-block;
  margin-top: 4px;

  user-select: none;
`

const BottomSeparator = styled.div`
  background-color: rgba(0, 0, 0, 0.05);
  height: 1px;
`

const BottomTabButtonLayout = styled.div`
  display: flex;
  flex-direction: column;
  align-items: center;

  padding: 4px;

  font-weight: 300;
  font-size: 10px;
  line-height: 11px;

  color: rgba(0, 0, 0, 0.55);
  user-select: none;

  &[data-selected="true"] {
    background-color: rgba(0, 0, 0, 0.05);
  }

  & > *:nth-child(1) {
    color: var(--color-primary);
  }

  & > *:nth-child(2) {
    margin-top: 4px;
  }

  &:hover[data-selected="false"] {
    background-color: rgba(0, 0, 0, 0.02);
  }

  transition: all 100ms ease-out;
`
const BottomTabButton = ({ text, icon, selected, ...props }) => {
  return (
    <BottomTabButtonLayout data-selected={selected} {...props}>
      <IconShell icon={icon} />
      <span>{text}</span>
    </BottomTabButtonLayout>
  )
}

const BottomSection = styled.section`
  display: flex;
  justify-content: stretch;

  overflow: hidden;

  & > * {
    flex-grow: 1;
  }

  border-bottom-left-radius: 10px;
  border-bottom-right-radius: 10px;
`

const ComputerInfo = styled.div`
  display: grid;
  margin-top: 24px;
  grid-template-columns: 1fr;
  grid-template-rows: repeat(4, min-content);
  grid-row-gap: 20px;

  color: rgba(0, 0, 0, 0.7);
  font-weight: lighter;
`

const InfoDetailLayout = styled.section`
  display: grid;
  grid-template-columns: 12px 1fr;
  grid-column-gap: 20px;

  & > *:first-child {
    grid-column: 1;
    grid-row: span;
  }

  & > *:not(:first-child) {
    grid-column: 2;
  }

  user-select: none;
`
const InfoDetail = ({ icon, iconColor, children }) => {
  return (
    <InfoDetailLayout>
      <IconShell icon={icon} style={{ color: iconColor }} />
      <div>{children}</div>
    </InfoDetailLayout>
  )
}

const FileSystemLayout = styled.section`
  display: flex;
  margin-top: 8px;
  flex-direction: column;
  overflow: auto;
`

const FileItemLayout = styled.article`
  display: grid;
  grid-template-rows: 24px;
  grid-template-columns: 16px auto 1fr auto;
  column-gap: 8px;
  align-items: center;

  margin-left: ${({ depth }) => `${depth * 12}px;`};

  &:hover[data-selected="false"] {
    background-color: rgba(0, 0, 0, 0.04);
  }

  &[data-selected="true"] {
    background-color: rgba(0, 0, 0, 0.07);
  }
`
const FileItem = ({ depth, selected, name, type }) => {
  return (
    <FileItemLayout {...{ depth }} data-selected={selected}>
      <IconShell
        size="small"
        icon={type === "directory" ? FileDirectory : File}
        style={{
          color: Colors.primary,
          marginLeft: "8px",
        }}
      />
      <span
        style={{
          display: "inline-block",
          color: "rgba(0, 0, 0, 0.7)",
          fontSize: "16px",
          fontWeight: "lighter",
          marginTop: "4px",
          marginLeft: "8px",
          userSelect: "none",
        }}
      >
        {name}
      </span>
    </FileItemLayout>
  )
}

const EmptyFsLayout = styled.div`
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  user-select: none;

  color: rgba(0, 0, 0, 0.7);
`
const EmptyFs = ({}) => {
  return (
    <EmptyFsLayout>
      Obteniendo listado de archivos
      <LoadingIndicator />
    </EmptyFsLayout>
  )
}
